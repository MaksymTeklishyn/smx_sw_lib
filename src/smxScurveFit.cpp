#include "smxScurveFit.h"
#include "smxConstants.h"
#include <RooPlot.h>
#include <RooArgSet.h>
#include <RooMinimizer.h>
#include <TGaxis.h>
#include <TCanvas.h>
#include <TMath.h>
#include <TAxis.h>
#include <TPaveText.h>
#include <TRandom3.h> 
#include "TROOT.h" // Include general ROOT functionality
#include <iostream>

smxScurveFit::smxScurveFit(RooDataSet* dataset, int ch, int comp)
    : data(dataset),
      channel(ch),
      comparator(comp),
      readDiscList(), // Explicitly initialize as an empty vector
      pulseAmp(nullptr),
      countN(nullptr),
      countNorm(nullptr),
      offset(nullptr),
      threshold(nullptr),
      sigma(nullptr),
      fitModel(nullptr),
      fitResults(nullptr) {
    if (!data) {
        std::cerr << "Error: Null dataset passed to smxScurveFit constructor!" << std::endl;
        return;
    }

    // Initialize variables and the fit model
    initializeVariables();
    setupFitModel();
}


smxScurveFit::~smxScurveFit() {
    delete fitModel;
    delete fitResults;
}

void smxScurveFit::initializeVariables() {
    // Retrieve variables from the dataset
    pulseAmp = dynamic_cast<RooRealVar*>(data->get()->find("pulseAmp"));
    countN = dynamic_cast<RooRealVar*>(data->get()->find("countN"));
    countNorm = dynamic_cast<RooRealVar*>(data->get()->find("countNorm"));
    adcComp = dynamic_cast<RooCategory*>(data->get()->find("adcComp"));

    if (!pulseAmp || !countN || !countNorm || !adcComp) {
        std::cerr << "Error: Required variables not found in dataset!" << std::endl;
        return;
    }

    // Populate readDiscList from adcComp states
    readDiscList.clear();
    const auto& stateMap = adcComp->states();
    for (const auto& [name, value] : stateMap) {
        readDiscList.push_back(value);
    }

    // Initialize fit parameters
    offset = new RooRealVar("offset", "Offset", 0, -1., .5);
    threshold = new RooRealVar("threshold", "Threshold", 60.0, -1.0, 256.0);
    sigma = new RooRealVar("sigma", "Sigma", 1.0, .1, 15.0);
}

void smxScurveFit::randomizeInitialValues(double deviation) {
    TRandom3 rng; // Random number generator
    rng.SetSeed(0); // Use system time or a fixed value for reproducibility

    // Ensure deviation is non-negative
    if (deviation < 0) {
        std::cerr << "Error: Deviation must be non-negative." << std::endl;
        return;
    }

    // Generate random multipliers
    double offsetFactor = rng.Uniform(1.0 - deviation, 1.0 + deviation);
    double thresholdFactor = rng.Uniform(1.0 - deviation, 1.0 + deviation);
    double sigmaFactor = rng.Uniform(1.0 - deviation, 1.0 + deviation);

    // Calculate and clamp new values for offset
    double newOffset = offset->getVal() * offsetFactor;
    if (newOffset < offset->getMin()) newOffset = offset->getMin();
    if (newOffset > offset->getMax()) newOffset = offset->getMax();
    offset->setVal(newOffset);

    // Calculate and clamp new values for threshold
    double newThreshold = threshold->getVal() * thresholdFactor;
    if (newThreshold < threshold->getMin()) newThreshold = threshold->getMin();
    if (newThreshold > threshold->getMax()) newThreshold = threshold->getMax();
    threshold->setVal(newThreshold);

    // Calculate and clamp new values for sigma
    double newSigma = sigma->getVal() * sigmaFactor;
    if (newSigma < sigma->getMin()) newSigma = sigma->getMin();
    if (newSigma > sigma->getMax()) newSigma = sigma->getMax();
    sigma->setVal(newSigma);

    std::cout << "Randomized initial values with deviation " << deviation << ":" << std::endl;
    std::cout << "  Offset: " << offset->getVal() << " (clamped if necessary)" << std::endl;
    std::cout << "  Threshold: " << threshold->getVal() << " (clamped if necessary)" << std::endl;
    std::cout << "  Sigma: " << sigma->getVal() << " (clamped if necessary)" << std::endl;
}

void smxScurveFit::setupFitModel() {
    // Create the error function model
    fitModel = new RooFormulaVar(
        "fitModel",
        "offset + 0.5 * TMath::Erfc((threshold - pulseAmp) / (TMath::Sqrt(2) * sigma))",
        RooArgList(*pulseAmp, *offset, *threshold, *sigma)
    );
}

double smxScurveFit::fitScurvesSeq() {
    if (!data || !fitModel) {
        std::cerr << "Error: Dataset or model not initialized for fitting!" << std::endl;
        return -1.0;
    }

    RooArgSet variables(*offset, *threshold, *sigma, *adcComp);
    RooDataSet* fitResults = new RooDataSet("fitResults", "Fit results", variables, RooFit::StoreAsymError(variables));
    double totalChi2 = 0.0; // To accumulate chi2 values across all comparators
    int maxRetries = 3;

    for (int selectedDisc : readDiscList) {
        std::cout << "Fitting for comparator: " << selectedDisc << std::endl;

        RooDataSet* dataReduced = dynamic_cast<RooDataSet*>(data->reduce(Form("adcComp==%d", selectedDisc)));
        if (!dataReduced) {
            std::cerr << "Error: Failed to reduce dataset for comparator " << selectedDisc << "!" << std::endl;
            continue;
        }

        RooFitResult* result = nullptr;
        int retryCount = 0;

        while ((retryCount < maxRetries) && (!result || result->status() > 1)) {
 //         randomizeInitialValues(); // Randomize the initial values before each retry

            int strategy = (retryCount == 0) ? 0 : (retryCount == 1) ? 1 : 2; // Strategy adjustment
            std::cout << "Retry #" << retryCount << " with strategy " << strategy << "..." << std::endl;

            result = fitModel->chi2FitTo(
                *dataReduced,
                RooFit::YVar(*countNorm),
                RooFit::Save(),
                RooFit::Strategy(strategy),
                RooFit::PrintLevel(-1)
            );

            retryCount++;
        }

        if (result && result->status() <= 1) {
            std::cout << "Fit Results for comparator " << selectedDisc << ":" << std::endl;
            result->Print("v");
            fitResults->add(variables);
            totalChi2 += result->minNll(); // Accumulate chi2
        } else {
            std::cerr << "Fit failed for comparator " << selectedDisc << " after " << retryCount << " retries!" << std::endl;
        }

        // Clean up result and reduced dataset
        if (result) {
            delete result;
        }
        delete dataReduced;
    }

    std::cout << "Total Chi2: " << totalChi2 << std::endl;
    delete fitResults; // Clean up fit results dataset to avoid memory leaks
    return totalChi2;
}


TCanvas* smxScurveFit::drawPlot() const {
    TCanvas* canvas = new TCanvas("canvas", "S-Curve Fit", 1000, 400);

    if (!data || !pulseAmp || !countNorm || !fitModel) {
        std::cerr << "Error: Missing dataset, variables, or model for plotting." << std::endl;

        // Draw a dummy frame to indicate an error
        canvas->cd();
        canvas->SetFillColor(kWhite);
        TPaveText* errorText = new TPaveText(0.1, 0.4, 0.9, 0.6, "NDC");
        errorText->AddText("Error: Unable to generate plot.");
        errorText->SetFillColor(kRed - 10);
        errorText->SetTextColor(kBlack);
        errorText->SetTextFont(42);
        errorText->Draw();
        return canvas;
    }

    RooPlot* frame = pulseAmp->frame(RooFit::Title(" "));
    RooDataSet* dataReduced = dynamic_cast<RooDataSet*>(data->reduce("1"));
    dataReduced->plotOnXY(frame, RooFit::YVar(*countNorm), RooFit::DrawOption("PZ"), RooFit::MarkerStyle(7));
    fitModel->plotOn(frame, RooFit::LineWidth(1));

    // Customize axes
//  frame->GetXaxis()->SetTitle("Pulse Amplitude, a.u.");
    frame->GetXaxis()->SetNdivisions(16, false);
    frame->GetYaxis()->SetTitle("Normalized counts");
    frame->GetYaxis()->SetNdivisions(2);
    frame->Draw();

    // Draw the secondary axis
    TGaxis* secondaryAxis = new TGaxis(0, 105, 256, 105, 0, 256*smxAmCaltoE/1e3, 520, "-");
    secondaryAxis->SetTitle("Pulse charge (ke)");
    secondaryAxis->SetLabelSize(0.035); // Smaller label size
    secondaryAxis->SetTitleSize(0.035); // Smaller title size
    secondaryAxis->SetLabelFont(42);     // Standard ROOT font
    secondaryAxis->SetTitleFont(42);
    secondaryAxis->Draw();

    return canvas;
}


int smxScurveFit::getChannel() const {
    return channel;
}

int smxScurveFit::getComparator() const {
    return comparator;
}



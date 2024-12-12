#include "smxScurveFit.h"
#include "smxConstants.h"
#include <RooPlot.h>
#include <RooArgSet.h>
#include <RooChi2Var.h>
#include <RooMinimizer.h>
#include <TGaxis.h>
#include <TCanvas.h>
#include <TMath.h>
#include <TAxis.h>
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
    sigma = new RooRealVar("sigma", "Sigma", 3.0, 1.0, 15.0);
}

void smxScurveFit::setupFitModel() {
    // Create the error function model
    fitModel = new RooFormulaVar(
        "fitModel",
        "offset + 0.5 * TMath::Erfc((threshold - pulseAmp) / (TMath::Sqrt(2) * sigma))",
        RooArgList(*pulseAmp, *offset, *threshold, *sigma)
    );
}

double smxScurveFit::fitAllScurves() {
    if (!data || !fitModel) {
        std::cerr << "Error: Dataset or model not initialized for fitting!" << std::endl;
        return -1.0;
    }
    RooArgSet variables(*offset, *threshold, *sigma);
    RooDataSet* fitResults = new RooDataSet("fitResults", "Fit results", variables, RooFit::StoreAsymError(variables));

    int maxRetries = 5; // Maximum number of retries
    int retryCount = 0;

    int selectedDisc = readDiscList[2]; // Select a specific comparator
    RooDataSet* dataReduced = dynamic_cast<RooDataSet*>(data->reduce(Form("adcComp==%d", selectedDisc)));

    if (!dataReduced) {
        std::cerr << "Error: Failed to reduce dataset!" << std::endl;
        return -1.0;
    }

    offset->setVal(countNorm->getVal());

    RooFitResult* result = nullptr;
    do {
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
    } while ((result && result->status() > 1) && retryCount < maxRetries);

    if (result && result->status() <= 1) {
        std::cout << "Fit Results:" << std::endl;
        result->Print("v");
        fitResults->add(variables);
        return result->minNll();
    } else {
        std::cerr << "Fit failed after " << retryCount << " retries!" << std::endl;
        delete result;
        return -1.0;
    }
}

TCanvas* smxScurveFit::drawPlot() const {
    if (!data) {
        std::cerr << "Error: No RooDataSet available for plotting." << std::endl;
        return nullptr;
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

    // Create a canvas and draw the frame
    TCanvas* canvas = new TCanvas("canvas", "S-Curve Fit", 1000, 400);
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



#include "smxScurveFit.h"
#include "smxConstants.h"
#include <RooPlot.h>
#include <TCanvas.h>
#include <TFile.h>
#include <TString.h>
#include <TGaxis.h>
#include <TAxis.h>
#include <TMath.h>
#include <RooChi2Var.h>
#include <RooMinimizer.h>
#include <RooRealVar.h>
#include <RooFormulaVar.h>
#include <RooPlot.h>

smxScurveFit::smxScurveFit(RooDataSet* dataset, int ch, int comp)
    : data(dataset), channel(ch), comparator(comp), fitResult(nullptr),
      pulseAmp(nullptr), efficiency(nullptr), scurvePdf(nullptr) {
    if (!data) {
        std::cerr << "Error: Null dataset passed to smxScurveFit constructor!" << std::endl;
        return;
    }

    // Initialize RooFit variables and the S-curve model
    setupModel();
}

smxScurveFit::~smxScurveFit() {
    delete pulseAmp;
    delete efficiency;
    delete scurvePdf;
    delete fitResult;
}

TCanvas* smxScurveFit::drawPlot(const TString& outputFilename) const {
    if (!data) {
        std::cerr << "Error: No RooDataSet available for plotting." << std::endl;
        return nullptr;
    }
        
    // Retrieve the RooRealVars for pulse amplitude and countN
    RooRealVar* pulseAmp = (RooRealVar*)data->get()->find("pulseAmp");
    RooRealVar* countN = (RooRealVar*)data->get()->find("countN");
    
    if (!pulseAmp || !countN) {
        std::cerr << "Error: Variables 'pulseAmp' or 'countN' not found in the dataset." << std::endl;
        return nullptr;
    }
        
    // Create a frame for the x-axis variable (pulseAmp)
    RooPlot* frame = pulseAmp->frame(RooFit::Title(" "));

    // Plot the dataset on the frame
    data->plotOnXY(frame, RooFit::YVar(*countN), RooFit::DrawOption("PZ"), RooFit::MarkerStyle(7));

    // If a fit was performed, overlay the S-curve fit
    if (fitResult) {
        scurvePdf->plotOn(frame);
    } else {
        std::cerr << "Warning: No fit has been performed. Plotting only data points." << std::endl;
    }   

    frame->GetXaxis()->SetTitle("Pulse Amplitude, a.u.");
    frame->GetXaxis()->SetNdivisions(16, false);
    frame->GetYaxis()->SetTitle("Count");
    
    // Create a TCanvas and draw the frame
    TCanvas* canvas = new TCanvas("canvas", "S-Curve Fit", 1000, 400);
    frame->Draw();

    // Draw the secondary axis
    TGaxis* secondaryAxis = new TGaxis(0, 105, 256, 105, 0, 256*smxAmCaltoE/1e3, 520, "-");
    secondaryAxis->SetTitle("Pulse Amplitude, ke");
    secondaryAxis->SetLabelSize(0.035); // Smaller label size
    secondaryAxis->SetTitleSize(0.035); // Smaller title size
    secondaryAxis->SetLabelFont(42);     // Standard ROOT font
    secondaryAxis->SetTitleFont(42);
    secondaryAxis->Draw();

    // Save the plot to the specified file
    canvas->SaveAs(outputFilename);
    return canvas; // Return the TCanvas for further use
}


void smxScurveFit::setupModel() {
    // Define variables
    pulseAmp = new RooRealVar("pulseAmp", "Pulse Amplitude", -10, 300);
    efficiency = new RooRealVar("efficiency", "Efficiency", 0.0, 1.0);

    // Define S-curve model (sigmoid: 1 / (1 + exp(-a * (x - b))))
    RooRealVar* a = new RooRealVar("a", "Slope", 0.1, 0.0, 10.0);
    RooRealVar* b = new RooRealVar("b", "Threshold", 50.0, -10.0, 300.0);
    scurvePdf = new RooGenericPdf("scurvePdf", "1 / (1 + exp(-a * (pulseAmp - b)))",
                                  RooArgList(*pulseAmp, *a, *b));
}

RooFitResult* smxScurveFit::performFit() {
    if (!data || !scurvePdf) {
        std::cerr << "Error: Dataset or PDF not initialized for fitting!" << std::endl;
        return nullptr;
    }

    // Perform the fit
    fitResult = scurvePdf->fitTo(*data, RooFit::Save());
    return fitResult;
}

void smxScurveFit::plotFitResults(const TString& outputFilename) const {
    if (!data || !pulseAmp || !scurvePdf) {
        std::cerr << "Error: Data or model not initialized for plotting!" << std::endl;
        return;
    }

    // Create a frame for pulseAmp
    RooPlot* frame = pulseAmp->frame(RooFit::Title("S-Curve Fit"));
    data->plotOn(frame);
    scurvePdf->plotOn(frame);

    // Draw the frame on a canvas
    TCanvas canvas("canvas", "S-Curve Fit", 800, 600);
    frame->Draw();
    canvas.SaveAs(outputFilename);
}

void smxScurveFit::printFitResults() const {
    if (!fitResult) {
        std::cerr << "Error: No fit results available to print!" << std::endl;
        return;
    }

    fitResult->Print("v");
}

int smxScurveFit::getChannel() const {
    return channel;
}

int smxScurveFit::getComparator() const {
    return comparator;
}


double smxScurveFit::fitErrFunction() {
    if (!data) {
        std::cerr << "Error: No dataset available for fitting!" << std::endl;
        return -1.0;
    }

    RooRealVar* pulseAmp = (RooRealVar*)data->get()->find("pulseAmp");
    RooRealVar* countN = (RooRealVar*)data->get()->find("countN");

    if (!pulseAmp || !countN) {
        std::cerr << "Error: Variables 'pulseAmp' or 'countN' not found in the dataset." << std::endl;
        return -1.0;
    }

    // Define parameters for the error function model
    RooRealVar amplitude("amplitude", "Amplitude", 1.0, 0.0, 300.0);
    RooRealVar threshold("threshold", "Threshold", 50.0, -10.0, 300.0);
    RooRealVar sigma("sigma", "Sigma", 10.0, 0.1, 100.0);

    // Define the error function model using RooFormulaVar
    RooFormulaVar errorFunc(
        "errorFunc",
        "@0 * 0.5 * TMath::Erfc( ( @1 - @2 ) / (TMath::Sqrt(2) * @3) )",
        RooArgList(amplitude, *pulseAmp, threshold, sigma)
    );
    errorFunc.chi2FitTo(*data, RooFit::YVar(*countN));

    // Retrieve parameter values
    double fittedAmplitude = amplitude.getVal();
    double fittedThreshold = threshold.getVal();
    double fittedSigma = sigma.getVal();

    // Print results
    std::cout << "Fit Results:" << std::endl;
    std::cout << "  Amplitude: " << fittedAmplitude << " ± " << amplitude.getError() << std::endl;
    std::cout << "  Threshold: " << fittedThreshold << " ± " << threshold.getError() << std::endl;
    std::cout << "  Sigma:     " << fittedSigma << " ± " << sigma.getError() << std::endl;

    return 0;
}


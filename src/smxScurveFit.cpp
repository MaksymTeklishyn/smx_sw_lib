#include "smxScurveFit.h"
#include "smxConstants.h"
#include <RooPlot.h>
#include <RooChi2Var.h>
#include <RooMinimizer.h>
#include <TGaxis.h>
#include <TCanvas.h>
#include <TMath.h>
#include <TAxis.h>
#include <iostream>

smxScurveFit::smxScurveFit(RooDataSet* dataset, int ch, int comp)
    : data(dataset), channel(ch), comparator(comp),
      pulseAmp(nullptr), amplitude(nullptr), threshold(nullptr), sigma(nullptr),
      fitModel(nullptr), fitResult(nullptr) {
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
    delete fitResult;
}

void smxScurveFit::initializeVariables() {
    // Retrieve variables from the dataset, or create them if they don't exist
    pulseAmp = (RooRealVar*)data->get()->find("pulseAmp");
    if (!pulseAmp) {
        pulseAmp = new RooRealVar("pulseAmp", "Pulse Amplitude", -10, 300);
    }
    countN = (RooRealVar*)data->get()->find("countN");
    if (!countN) {
        RooRealVar countN("countN", "Comparator counts", 0, 300); // Range of counts
    }

    amplitude = new RooRealVar("amplitude", "Amplitude", 100.0, 0.0, 300.0);
    threshold = new RooRealVar("threshold", "Threshold", 60.0, -10.0, 300.0);
    sigma = new RooRealVar("sigma", "Sigma", 3.0, 0.1, 100.0);
}

void smxScurveFit::setupFitModel() {
    // Create the error function model
    fitModel = new RooFormulaVar(
        "fitModel",
        "@0 * 0.5 * TMath::Erfc((@2 - @1) / (TMath::Sqrt(2) * @3))",
        RooArgList(*amplitude, *pulseAmp, *threshold, *sigma)
    );
}

double smxScurveFit::fitErrFunction() {
    if (!data || !fitModel) {
        std::cerr << "Error: Dataset or model not initialized for fitting!" << std::endl;
        return -1.0;
    }
/*
    RooRealVar* countN = (RooRealVar*)data->get()->find("countN");
    if (!countN) {
        std::cerr << "Error: Variable 'countN' not found in the dataset." << std::endl;
        return -1.0;
    }
*/
    // Perform the chi-square fit
    RooFitResult* result = fitModel->chi2FitTo(*data, RooFit::YVar(*countN), RooFit::Save());

    if (result) {
        fitResult = result;
        std::cout << "Fit Results:" << std::endl;
        fitResult->Print("v");
        return fitResult->minNll();
    } else {
        std::cerr << "Fit failed!" << std::endl;
        return -1.0;
    }
}

TCanvas* smxScurveFit::drawPlot(const TString& outputFilename) const {
    if (!data) {
        std::cerr << "Error: No RooDataSet available for plotting." << std::endl;
        return nullptr;
    }

    // Create a frame for the pulse amplitude
    RooPlot* frame = pulseAmp->frame(RooFit::Title(" "));

//  RooRealVar* countN = (RooRealVar*)data->get()->find("countN");
    // Plot the dataset on the frame
    data->plotOnXY(frame, RooFit::YVar(*countN), RooFit::DrawOption("PZ"), RooFit::MarkerStyle(7));
    fitModel->plotOn(frame, RooFit::LineWidth(1));

    // Customize axes
    frame->GetXaxis()->SetTitle("Pulse Amplitude, a.u.");
    frame->GetXaxis()->SetNdivisions(16, false);
    frame->GetYaxis()->SetTitle("Count");

    // Create a canvas and draw the frame
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

    return canvas;
}

int smxScurveFit::getChannel() const {
    return channel;
}

int smxScurveFit::getComparator() const {
    return comparator;
}

void smxScurveFit::printFitResults() const {
    if (!fitResult) {
        std::cerr << "Error: No fit results available to print!" << std::endl;
        return;
    }

    fitResult->Print("v");
}


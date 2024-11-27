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
      pulseAmp(nullptr), countN(nullptr), countNorm(nullptr), offset(nullptr), threshold(nullptr), sigma(nullptr),
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
    countN = (RooRealVar*)data->get()->find("countN");
    countNorm = (RooRealVar*)data->get()->find("countNorm");
    adcComp = (RooCategory*)data->get()->find("adcComp");

    offset = new RooRealVar("offset", "Offset", .0, -2., 2.0);
    threshold = new RooRealVar("threshold", "Threshold", 60.0, -10.0, 300.0);
    sigma = new RooRealVar("sigma", "Sigma", 3.0, 0.1, 100.0);
}

void smxScurveFit::setupFitModel() {
    // Create the error function model
    fitModel = new RooFormulaVar(
        "fitModel",
        "@0 + 0.5 * TMath::Erfc((@2 - @1) / (TMath::Sqrt(2) * @3))",
        RooArgList(*offset, *pulseAmp, *threshold, *sigma)
    );
}

double smxScurveFit::fitErrFunction() {
    if (!data || !fitModel) {
        std::cerr << "Error: Dataset or model not initialized for fitting!" << std::endl;
        return -1.0;
    }

    RooDataSet* dataReduced = dynamic_cast<RooDataSet*>(data->reduce("adcComp==16"));
    RooFitResult* result = fitModel->chi2FitTo(*dataReduced, RooFit::YVar(*countNorm), RooFit::Save());

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


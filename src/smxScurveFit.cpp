#include "smxScurveFit.h"
#include <RooPlot.h>
#include <TCanvas.h>
#include <TFile.h>
#include <TString.h>

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


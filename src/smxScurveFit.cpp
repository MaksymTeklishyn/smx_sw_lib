#include "smxScurveFit.h"
#include <RooPlot.h>
#include <TCanvas.h>
#include <TFile.h>
#include <TString.h>
#include <TAxis.h>
#include <TMath.h>

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
    RooPlot* frame = pulseAmp->frame(RooFit::Title("S-Curve Fit"));

    // Plot the dataset on the frame
    data->plotOnXY(frame, RooFit::YVar(*countN), RooFit::DrawOption("PZ") ,  RooFit::MarkerStyle(7));

    // If a fit was performed, overlay the S-curve fit
    if (fitResult) {
        scurvePdf->plotOn(frame);
    } else {
        std::cerr << "Warning: No fit has been performed. Plotting only data points." << std::endl;
    }

    // Label the axes
    frame->GetXaxis()->SetTitle("Pulse Amplitude");
    frame->GetYaxis()->SetTitle("Count");

    // Create a TCanvas and draw the frame
    TCanvas* canvas = new TCanvas("canvas", "S-Curve Fit", 1000, 500);
    frame->Draw();

    // Save the plot to the specified file
    canvas->SaveAs(outputFilename);

    std::cout << "Plot saved as: " << outputFilename << std::endl;

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


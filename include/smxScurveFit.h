#ifndef SMX_SCURVE_FIT_H
#define SMX_SCURVE_FIT_H

#include <RooDataSet.h>
#include <RooRealVar.h>
#include <RooCategory.h>
#include <RooFormulaVar.h>
#include <RooFitResult.h>
#include <TString.h>
#include <TCanvas.h>
#include <iostream>
#include <vector>

/**
 * @class smxScurveFit
 * @brief Class for fitting S-curve data using RooFit, specifically with an error function (erfc) model.
 */
class smxScurveFit {
private:
    RooDataSet* data;           ///< Pointer to the RooDataSet for fitting.
    int channel;                ///< Channel number, -1 if unknown.
    int comparator;             ///< Comparator number, -1 if unknown.
    std::vector<int> readDiscList;  ///< Positions of discriminators

    RooRealVar* pulseAmp;       ///< Pointer to the pulse amplitude variable.
    RooRealVar* countN;   
    RooRealVar* countNorm;   
    RooCategory* adcComp;
 
    RooRealVar* offset;         ///< Pointer to the amplitude parameter.
    RooRealVar* threshold;      ///< Pointer to the threshold parameter.
    RooRealVar* sigma;          ///< Pointer to the sigma parameter.

    RooFormulaVar* fitModel;    ///< Pointer to the error function model used for fitting.

    RooDataSet* fitResults;     ///< Pointer to the resulted variables of the fits.

    /**
     * @brief Initialize all variables and the model for the error function fit.
     * @details Retrieves variables from the dataset if they exist, or initializes them otherwise.
     */
    void initializeVariables();

    /**
     * @brief Initialize the error function model.
     * @details Sets up the `fitModel` using `RooFormulaVar` and the initialized variables.
     */
    void setupFitModel();

public:
    /**
     * @brief Constructor for smxScurveFit.
     * @param dataset Pointer to the RooDataSet for fitting.
     * @param ch Channel number (-1 if unknown).
     * @param comp Comparator number (-1 if unknown).
     */
    smxScurveFit(RooDataSet* dataset, int ch = -1, int comp = -1);

    /**
     * @brief Destructor to clean up dynamically allocated memory.
     */
    ~smxScurveFit();

    /**
     * @brief Performs a chi-square fit using an error function model (erfc).
     * @return The chi-square value of the fit, or -1 on error.
     */
    double fitAllScurves();

    /**
     * @brief Generates and returns a TCanvas with the S-curve fit plot.
     * @details Overlays the fit result on the dataset and optionally saves it as a PDF.
     * @param outputFilename Name of the output PDF file for the plot (default: "testDataSet.pdf").
     * @return Pointer to the TCanvas object containing the plot.
     */
    TCanvas* drawPlot() const;

    // Getters
    /**
     * @brief Get the channel number.
     * @return The channel number.
     */
    int getChannel() const;

    /**
     * @brief Get the comparator number.
     * @return The comparator number.
     */
    int getComparator() const;

};

#endif // SMX_SCURVE_FIT_H


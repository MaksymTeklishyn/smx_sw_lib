#ifndef SMX_SCURVE_FIT_H
#define SMX_SCURVE_FIT_H

#include <RooDataSet.h>
#include <RooRealVar.h>
#include <RooGenericPdf.h>
#include <RooFitResult.h>
#include <TString.h>
#include <iostream>

/**
 * @class smxScurveFit
 * @brief Class to handle S-curve fitting using RooFit classes.
 */
class smxScurveFit {
private:
    RooDataSet* data;          ///< Pointer to the RooDataSet for fitting
    int channel;               ///< Channel number
    int comparator;            ///< Comparator number
    RooFitResult* fitResult;   ///< Pointer to the fit result (to manage memory)

    // RooFit variables and models
    RooRealVar* pulseAmp;      ///< Variable representing pulse amplitude
    RooRealVar* efficiency;    ///< Variable representing efficiency
    RooGenericPdf* scurvePdf;  ///< S-curve model

    /**
     * @brief Sets up the S-curve model for fitting.
     */
    void setupModel();

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
     * @brief Performs the fitting procedure.
     * @return Pointer to the RooFitResult (ownership is retained by the class).
     */
    RooFitResult* performFit();

    /**
     * @brief Plots the fit results to a file.
     * @param outputFilename Name of the output file for the plot.
     */
    void plotFitResults(const TString& outputFilename) const;

    /**
     * @brief Prints the fit results to the console.
     */
    void printFitResults() const;

    // Getters
    int getChannel() const;
    int getComparator() const;
};

#endif // SMX_SCURVE_FIT_H


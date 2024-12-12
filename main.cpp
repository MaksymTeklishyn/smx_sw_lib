#include "smxPscan.h"
#include "smxScurveFit.h"
#include "smxAsic.h"
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <filename>" << std::endl;
        return 1;
    }

    std::string filename = argv[1];
    smxPscan* pscan = new smxPscan(); 

    pscan->readAsciiFile(filename)->Print();
    pscan->writeRootFile();
    smxScurveFit* scurveFit;
    TCanvas* canvA = new TCanvas("canvA", "S-Curve Fit", 1000, 400);
    canvA->Print("testDataSet.pdf[");
//  for (int i=0; i<smxNCh; ++i) {
    for (int i=0; i<10; ++i) {
        scurveFit = new smxScurveFit(pscan->toRooDataSet(i));
        scurveFit->fitAllScurves();
        scurveFit->drawPlot()->Print("testDataSet.pdf");
        delete scurveFit;
    }
    canvA->Print("testDataSet.pdf]");
//  smxAsic asic;       
//  asic.addPscan(pscan);
                        
                        
    return 0;           
}                       


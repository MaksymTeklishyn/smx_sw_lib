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
    smxScurveFit* scurveFit = new smxScurveFit(pscan->toRooDataSet(102));
    scurveFit->fitErrFunction();
    scurveFit->drawPlot();
//  smxAsic asic;       
//  asic.addPscan(pscan);
                        
                        
    return 0;           
}                       


#include "smxPscan.h"
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
    smxAsic asic;       
    asic.addPscan(pscan);
                        
                        
    return 0;           
}                       


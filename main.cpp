#include "smxPscan.h"
#include <iostream>

int main(int argc, char* argv[]) {
    // Check if a filename was provided as an argument
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <filename>" << std::endl;
        return 1;
    }

    // Get the filename from the command-line arguments
    std::string filename = argv[1];

    // Create an instance of the smxPscan class and read the file
    smxPscan pscan;
    TTree* tree = pscan.readAsciiFile(filename);

    if (tree) {
        // Print the TTree structure for verification
        tree->Print();
    } else {
        std::cerr << "Failed to process the file: " << filename << std::endl;
    }

    pscan.writeRootFile(); // Optional argument: specify a custom output file name
    return 0;
}













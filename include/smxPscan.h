#ifndef SMX_PSCAN_H
#define SMX_PSCAN_H

#include <TTree.h>
#include <TObject.h> // Include TObject for ROOT I/O compatibility
#include <fstream>
#include <string>
#include <vector>
#include <ctime>  // For std::time_t and std::tm

class smxPscan : public TObject {
private:
    TTree* pscanTree;                   // Internal field to store the TTree
    std::string asciiFileName;          // Field to store the ASCII file name
    std::string asciiFileAddress;       // Field to store the ASCII file address
    std::ifstream asciiFile;            // Field for the input file stream
    std::vector<int> readDiscList;      // Field to store DISC_LIST positions

    // Parsed data fields
    std::time_t readTime;               // Parsed read time as a numeric value (epoch time)
    std::string asicId;                 // Parsed ASIC ID (e.g., "XA-000-08-002-000-002-205-02")
    int nPulses;                        // Parsed number of pulses (e.g., 100)

    // Helper function to parse the first line and populate readDiscList
    void parseHeaderLine(const std::string& line);

    // Helper function to parse the asciiFileName
    void parseAsciiFileName();

    // Helper function to convert readTime to a human-readable string
    std::string formatReadTime() const;

public:
    // Constructor to initialize the TTree
    smxPscan();

    // Destructor to manage memory and close the file if necessary
    ~smxPscan();

    // Method to read an ASCII file and fill the TTree
    TTree* readAsciiFile(const std::string& filename);

    // Method to write the TTree to a ROOT file
    void writeRootFile(const std::string& outputFileName = "");

    // Getter to access the internal TTree
    TTree* getTree() const;

    // Getters for the file name and address
    std::string getAsciiFileName() const;
    std::string getAsciiFileAddress() const;

    // Getter for read DISC_LIST positions
    const std::vector<int>& getReadDiscList() const;

    // Getters for parsed data fields
    std::time_t getReadTime() const;
    std::string getAsicId() const;
    int getNPulses() const;
};

#endif // SMX_PSCAN_H


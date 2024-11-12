#include "smxPscan.h"
#include <TFile.h>
#include <TNamed.h>
#include <TParameter.h>
#include <iostream>
#include <sstream>
#include <regex>
#include <filesystem> // For handling file paths

// Constructor to initialize the TTree
smxPscan::smxPscan() : pscanTree(new TTree("pscanTree", "Tree for pulse scan data")) {}

// Destructor to manage memory and close the file if necessary
smxPscan::~smxPscan() {
    delete pscanTree;
    if (asciiFile.is_open()) {
        asciiFile.close();
    }
}

void smxPscan::parseHeaderLine(const std::string& line) {
    std::regex disc_list_regex(R"(\bDISC_LIST:\[(.*?)\])");
    std::smatch match;
    std::vector<int> discPositions;

    if (std::regex_search(line, match, disc_list_regex)) {
        std::stringstream ss(match[1]);
        int pos;
        while (ss >> pos) {
            discPositions.push_back(pos);
            if (ss.peek() == ',') ss.ignore();
        }

        // Resize readDiscList to match discPositions and copy elements
        readDiscList.Set(discPositions.size());
        for (size_t i = 0; i < discPositions.size(); ++i) {
            readDiscList[i] = discPositions[i];
        }
    }

    // Debugging output to confirm positions
    std::cout << "Parsed DISC_LIST positions: ";
    for (int i = 0; i < readDiscList.GetSize(); ++i) {
        std::cout << readDiscList[i] << " ";
    }
    std::cout << std::endl;
}

// Helper function to parse the asciiFileName
void smxPscan::parseAsciiFileName() {
    std::regex filename_regex(R"(pscan_(\d{6}_\d{4})_(XA-[\d\-]+)_.*_NP_(\d+)_.*\.txt)");
    std::smatch match;

    if (std::regex_search(asciiFileName, match, filename_regex)) {
        std::string readTimeStr = match[1]; // Extract read time as a string
        asicId = match[2];                  // Extract ASIC ID
        nPulses = std::stoi(match[3]);      // Extract number of pulses

        // Parse the readTime string into a std::tm struct
        std::tm timeStruct = {};
        timeStruct.tm_year = std::stoi(readTimeStr.substr(0, 2)) + 100; // Years since 1900
        timeStruct.tm_mon = std::stoi(readTimeStr.substr(2, 2)) - 1;    // Month is 0-based
        timeStruct.tm_mday = std::stoi(readTimeStr.substr(4, 2));
        timeStruct.tm_hour = std::stoi(readTimeStr.substr(7, 2));
        timeStruct.tm_min = std::stoi(readTimeStr.substr(9, 2));
        timeStruct.tm_sec = 0; // Default to 0 seconds

        // Convert std::tm to std::time_t (epoch time)
        readTime = std::mktime(&timeStruct);
        if (readTime == -1) {
            logError("Failed to convert time.");
        }
    }

    // Debugging output to print parsed fields
    std::cout << "readTime: " << readTime << " (" << formatReadTime() << ")" << std::endl;
    std::cout << "asicId: " << asicId << std::endl;
    std::cout << "nPulses: " << nPulses << std::endl;
}

// Helper function to convert readTime to a human-readable string
std::string smxPscan::formatReadTime() const {
    if (readTime == 0) return "Invalid time";

    // Convert std::time_t to std::tm for formatting
    std::tm* timeStruct = std::localtime(&readTime);
    std::ostringstream oss;
    oss << std::put_time(timeStruct, "%d %B %Y %H:%M");

    return oss.str();
}

// Method to log errors for consistency
void smxPscan::logError(const std::string& message) const {
    std::cerr << "Error: " << message << std::endl;
}

// Helper function to generate default output file name based on ASCII file name
std::string smxPscan::generateDefaultOutputFileName() const {
    std::filesystem::path filePath(asciiFileName);
    std::string baseName = filePath.stem().string();
    return asciiFileAddress + "/" + baseName + "_output.root";
}

// Method to read an ASCII file and fill the TTree
TTree* smxPscan::readAsciiFile(const std::string& filename) {
    // Use std::filesystem to extract the file name and path
    std::filesystem::path filePath(filename);
    asciiFileName = filePath.filename().string();
    asciiFileAddress = filePath.parent_path().string();
    parseAsciiFileName();

    // Open the text file
    asciiFile.open(filename);
    if (!asciiFile.is_open()) {
        logError("Opening file: " + filename);
        return nullptr;
    }

    // Parse header line to extract DISC_LIST positions
    std::string line;
    std::getline(asciiFile, line);
    parseHeaderLine(line);

    // Variables for TTree branches
    int pulse, channel;
    int adc[smxNAdc] = {0};  // Array of fixed size smxNAdc, initialized to 0
    int tcomp;          // Timing comparator

    // Set branch addresses
    pscanTree->Branch("pulse", &pulse, "pulse/I");
    pscanTree->Branch("channel", &channel, "channel/I");
    pscanTree->Branch("ADC", adc, Form("ADC[%d]/I", smxNAdc));
    pscanTree->Branch("tcomp", &tcomp, "tcomp/I");

    // Regex pattern to parse each data line
    std::regex data_pattern(R"(vp\s+(\d+)\s+ch\s+(\d+):\s+((\d+\s*)+))");

    // Read and parse each line of data
    while (std::getline(asciiFile, line)) {
        std::smatch data_match;
        if (std::regex_match(line, data_match, data_pattern)) {
            // Extract pulse and channel
            pulse = std::stoi(data_match[1]);
            channel = std::stoi(data_match[2]);

            // Reset adc array to zero for each entry
            std::fill(std::begin(adc), std::end(adc), 0);

            // Extract ADC values into a string and parse them into the array
            std::string adc_values_str = data_match[3];
            std::istringstream iss(adc_values_str);
            int value;
            size_t index = 0;

            // Read ADC values up to the second-to-last value for adc, and assign the last to tcomp
            while (iss >> value) {
                if (index < readDiscList.GetSize() - 1 && readDiscList[index] < smxNAdc) {
                    adc[readDiscList[index]] = value;
                } else {
                    tcomp = value;  // Last value as timing comparator
                }
                ++index;
            }

            // Fill the TTree
            pscanTree->Fill();
        } else {
            logError("Failed to match the line: " + line);
        }
    }

    // Close the file
    asciiFile.close();

    // Return the filled TTree
    return pscanTree;
}

// Method to write the TTree and metadata to a ROOT file
void smxPscan::writeRootFile(const std::string& outputFileName) {
    std::string outputFile = outputFileName.empty() ? generateDefaultOutputFileName() : outputFileName;
    TFile file(outputFile.c_str(), "RECREATE");

    if (file.IsOpen()) {
        pscanTree->Write();
	// Write metadata:
        file.WriteObject(&asicId, "asicId");
        file.WriteObject(&readDiscList, "readDiscList");
        TParameter<int> nPulsesParam("nPulses", nPulses);
        nPulsesParam.Write();

        file.Close();
        std::cout << "File written successfully to: " << outputFile << std::endl;
    } else {
        logError("Failed to create output file: " + outputFile);
    }
}


// Getter to access the internal TTree
TTree* smxPscan::getDataTree() const {
    return pscanTree;
}

// Getter for the ASCII file name
std::string smxPscan::getAsciiFileName() const {
    return asciiFileName;
}

// Getter for the ASCII file address
std::string smxPscan::getAsciiFileAddress() const {
    return asciiFileAddress;
}

// Getter for read DISC_LIST positions
const TArrayI& smxPscan::getReadDiscList() const {
    return readDiscList;
}

// Setter for the ASIC settings
void smxPscan::setAsicSettings(const smxAsicSettings& settings) {
    asicSettings = settings;
}

// Getter for the ASIC settings
smxAsicSettings& smxPscan::getAsicSettings() {
    return asicSettings;
}

#include "smxPscan.h"
#include <TFile.h>
#include <TCanvas.h>
#include <TAxis.h>
#include <TNamed.h>
#include <TParameter.h>
#include <RooRealVar.h>
#include <RooDataSet.h>
#include <RooArgSet.h>
#include <RooPlot.h>
#include <iostream>
#include <sstream>
#include <regex>
#include <filesystem> // For handling file paths

// Constructor to initialize the TTree
smxPscan::smxPscan() : pscanTree(new TTree("pscanTree", "Tree for pulse scan data")) {}

// Destructor to manage memory and close the file if necessary
smxPscan::~smxPscan() {
    delete pscanTree;
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
    // Updated regex pattern to capture Vref_p, Vref_n, Vref_t, and Thr2_glb
    std::regex filename_regex(R"(pscan_(\d{6}_\d{4})_(XA-[\d\-]+)_.*_SET_(\d+)_(\d+)_(\d+)_(\d+)_.*_NP_(\d+)_.*\.txt)");
    std::smatch match;

    if (std::regex_search(asciiFileName, match, filename_regex)) {
        std::string readTimeStr = match[1];     // Extract read time as a string
        asicId = match[2];                      // Extract ASIC ID
        int Vref_p = std::stoi(match[3]);       // Vref_p
        int Vref_n = std::stoi(match[4]);       // Vref_n
        int Vref_t = std::stoi(match[5]);       // Vref_t
        int Thr2_glb = std::stoi(match[6]);     // Thr2_glb
        nPulses = std::stoi(match[7]);          // Extract number of pulses

        // Update smxAsicSettings with extracted values
        asicSettings.setVref_p(Vref_p);
        asicSettings.setVref_n(Vref_n);
        asicSettings.setVref_t(Vref_t);
        asicSettings.setThr2_glb(Thr2_glb);

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
    std::cout << "Vref_p: " << asicSettings.getVref_p() << ", Vref_n: " << asicSettings.getVref_n()
              << ", Vref_t: " << asicSettings.getVref_t() << ", Thr2_glb: " << asicSettings.getThr2_glb() << std::endl;
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
    std::cout << "Processing file: " << asciiFileName << " at path: " << asciiFileAddress << std::endl;

    parseAsciiFileName();

    // Open the text file
    std::ifstream asciiFile;
    asciiFile.open(filename);
    if (!asciiFile.is_open()) {
        logError("Failed to open file: " + filename);
        return nullptr;
    }
    std::cout << "File opened successfully: " << filename << std::endl;

    // Parse header line to extract DISC_LIST positions
    std::string line;
    std::getline(asciiFile, line);
    std::cout << "Header line: " << line << std::endl;
    parseHeaderLine(line);

    // Variables for TTree branches
    int pulse, channel;
    int adc[smxNAdc] = {0};  // Array of fixed size smxNAdc, initialized to 0
    int tcomp;               // Timing comparator

    // Set branch addresses
    std::cout << "Setting up TTree branches..." << std::endl;
    pscanTree->Branch("pulse", &pulse, "pulse/I");
    pscanTree->Branch("channel", &channel, "channel/I");
    pscanTree->Branch("ADC", adc, Form("ADC[%d]/I", smxNAdc));
    pscanTree->Branch("tcomp", &tcomp, "tcomp/I");

    // Regex pattern to parse each data line
    std::regex data_pattern(R"(vp\s+(\d+)\s+ch\s+(\d+):\s+((\d+\s*)+))");

    // Read and parse each line of data
    Long64_t lineCount = 0;
    while (std::getline(asciiFile, line)) {
        lineCount++;
        std::smatch data_match;
        if (std::regex_match(line, data_match, data_pattern)) {
            // Extract pulse and channel
            pulse = std::stoi(data_match[1]);
            channel = std::stoi(data_match[2]);

            std::cout << "Line " << lineCount << ": Pulse: " << pulse << ", Channel: " << channel << std::endl;

            // Reset adc array to zero for each entry
            std::fill(std::begin(adc), std::end(adc), 0);

            // Extract ADC values into a string and parse them into the array
            std::string adc_values_str = data_match[3];
            std::istringstream iss(adc_values_str);
            std::fill(std::begin(adc), std::end(adc), 0);
            int value;
            int index = 0;
            // Read ADC values up to the second-to-last value for adc, and assign the last to tcomp
            while (iss >> value) {
                if (index < readDiscList.GetSize() - 1 && readDiscList[index] < smxNAdc) {
                    adc[readDiscList[index]] = value;
                    std::cout << "ADC[" << readDiscList[index] << "] = " << adc[readDiscList[index]] << "\t";
                } else {
                    tcomp = value;  // Last value as timing comparator
                    std::cout << "TComp = " << tcomp << std::endl;
                }
                ++index;
            }

            // Fill the TTree
            pscanTree->Fill();
            std::cout << "Filled TTree for line " << lineCount << std::endl;
        } else {
            logError("Failed to match the line: " + line);
        }
    }

    // Close the file
    asciiFile.close();
    std::cout << "File processing completed. Total lines: " << lineCount << std::endl;

    // Return the filled TTree
    return pscanTree;
}

// Method to write the TTree and metadata to a ROOT file
void smxPscan::writeRootFile(const std::string& outputFileName) {
    std::string outputFile = outputFileName.empty() ? generateDefaultOutputFileName() : outputFileName;
    TFile file(outputFile.c_str(), "RECREATE");

    if (file.IsOpen()) {
        pscanTree->Clone()->Write();
        settingsToTree()->Write();
        asicSettings.toTree()->Write("asicSettingsTree");

/*
        file.WriteObject(&asicId, "asicId");
        file.WriteObject(&readDiscList, "readDiscList");
        TParameter<int> nPulsesParam("nPulses", nPulses);
        nPulsesParam.Write();
*/
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

TString smxPscan::getAsicId() const {
    return asicId;
}

RooDataSet* smxPscan::toRooDataSet(int channelN, int comparator) const {
    // Check if the comparator index is valid
    if (comparator < 0 || comparator >= smxNAdc) {
        std::cerr << "Error: Comparator index out of range: " << comparator << std::endl;
        return nullptr;
    }

    // Step 1: Define RooRealVars for pulse amplitude (x-axis) and count number (y-axis)
    RooRealVar pulseAmp("pulseAmp", "Pulse Amplitude", -10, 300); // Range of pulse amplitudes
    RooRealVar countN("countN", "Count (Timing Comparator)", 0, 300); // Range of tcomp or count
    RooArgSet variables(pulseAmp, countN); // Group the variables into an ArgSet

    // Step 2: Create a new RooDataSet
    std::cout << "Creating RooDataSet for channel: " << channelN << " and comparator: " << comparator << std::endl;
    RooDataSet* dataset = new RooDataSet("pscanData", "Pulse vs TComp Data", variables);

    // Step 3: Check for required branches
    if (!pscanTree->GetBranch("pulse") || !pscanTree->GetBranch("channel") ||
        !pscanTree->GetBranch("ADC") || !pscanTree->GetBranch("tcomp")) {
        std::cerr << "Error: Required branches are missing from pscanTree." << std::endl;
        return nullptr;
    }

    std::cout << "Branches found: pulse, channel, ADC, tcomp." << std::endl;

    // Step 4: Set up branches for reading TTree data

    int pulse, channel, tcomp;
    int adc[smxNAdc] = {0}; // Ensures no garbage values
    pscanTree->SetBranchAddress("pulse", &pulse);
    pscanTree->SetBranchAddress("channel", &channel);
    pscanTree->SetBranchAddress("ADC", adc);
    pscanTree->SetBranchAddress("tcomp", &tcomp);

    std::cout << "Branch addresses set. Looping through TTree entries..." << std::endl;

    // Step 5: Loop over TTree entries and filter for the specified channel and comparator
    for (Long64_t i = 0; i < pscanTree->GetEntries(); ++i) {
        pscanTree->GetEntry(i);

        // Filter for the specified channel
        if (channel == channelN) {
            pulseAmp.setVal(pulse);           // Set x-axis variable
            countN.setVal(adc[comparator]);  // Set y-axis variable from the comparator

            // Debug: Show values being added to the dataset
            std::cout << "Adding to dataset - PulseAmp: " << pulseAmp.getVal()
                      << ", CountN: " << countN.getVal() << std::endl;

            dataset->add(variables);         // Add the entry to the dataset
        }
    }

    std::cout << "Finished creating RooDataSet. Total entries: " << dataset->numEntries() << std::endl;

    return dataset;
}

void smxPscan::plotRooDataSet(int channel, int comparator, const std::string& outputFilename) {
    // Generate the RooDataSet for the specified channel and comparator
    RooDataSet* dataset = toRooDataSet(channel, comparator);
    if (!dataset) {
        std::cerr << "Error: Failed to create RooDataSet." << std::endl;
        return;
    }

    // Retrieve the RooRealVars for pulse amplitude and count
    RooRealVar* pulseAmp = (RooRealVar*)dataset->get()->find("pulseAmp");
    RooRealVar* countN = (RooRealVar*)dataset->get()->find("countN");

    if (!pulseAmp || !countN) {
        std::cerr << "Error: Variables 'pulseAmp' or 'countN' not found in the dataset." << std::endl;
        delete dataset; // Clean up
        return;
    }

    // Create a frame for the x-axis variable (pulseAmp)
    RooPlot* frame = pulseAmp->frame(RooFit::Title("Pulse Amplitude vs Count"));

    // Plot the dataset on the frame with small dots and line connection
    dataset->plotOnXY(frame, RooFit::YVar(*countN), RooFit::MarkerStyle(kFullDotSmall), RooFit::LineStyle(kSolid));

    // Label the axes
    frame->GetXaxis()->SetTitle("Pulse Amplitude");
    frame->GetYaxis()->SetTitle("Count (Comparator)");

    // Create a TCanvas and draw the frame
    TCanvas canvas("canvas", "Pulse Amplitude vs Count", 1000, 500);
    frame->Draw();

    // Save the plot to the specified file
    canvas.SaveAs(outputFilename.c_str());

    std::cout << "Plot saved as: " << outputFilename << std::endl;

    // Clean up
    delete dataset;
}


void smxPscan::showTreeEntries() const {
    // Step 1: Check if required branches exist
    if (!pscanTree->GetBranch("pulse") || !pscanTree->GetBranch("channel") ||
        !pscanTree->GetBranch("ADC") || !pscanTree->GetBranch("tcomp")) {
        std::cerr << "Error: Required branches are missing from pscanTree." << std::endl;
        return;
    }

    std::cout << "Branches found: pulse, channel, ADC, tcomp." << std::endl;

    // Step 2: Set up branches for reading TTree data
    int pulse, channel, tcomp;
    int adc[smxNAdc] = {0}; // Array for ADC comparators
    pscanTree->SetBranchAddress("pulse", &pulse);
    pscanTree->SetBranchAddress("channel", &channel);
    pscanTree->SetBranchAddress("ADC", adc);
    pscanTree->SetBranchAddress("tcomp", &tcomp);

    std::cout << "Branch addresses set. Looping through TTree entries..." << std::endl;

    // Step 3: Loop over all TTree entries and print their contents
    for (Long64_t i = 0; i < pscanTree->GetEntries(); ++i) {
        pscanTree->GetEntry(i);

        // Print all variables for the current entry
        std::cout << "Entry: " << i
                  << " Channel: " << channel
                  << " Pulse: " << pulse
                  << " TComp: " << tcomp
                  << " ADC: ";
        for (int j = 0; j < smxNAdc; ++j) {
            std::cout << adc[j] << " ";
        }
        std::cout << std::endl;
    }

    std::cout << "Finished dumping all TTree entries." << std::endl;
}


TTree* smxPscan::settingsToTree() const {
    // Step 1: Create a new TTree instance
    TTree* tree = new TTree("pscanSettingsTree", "Settings Tree for SMX Pscan");

    // Persistent variables to ensure scope validity
    Long64_t readTimeLong = static_cast<Long64_t>(readTime);
    int nPulsesCopy = nPulses; // Copy to ensure persistence
    TString asicIdCopy = asicId; // Copy to ensure persistence
    std::vector<int> discListVec(readDiscList.GetSize());
    for (int i = 0; i < readDiscList.GetSize(); ++i) {
        discListVec[i] = readDiscList[i];
    }

    // Create branches with persistent variables
    tree->Branch("readTime", &readTimeLong, "readTime/L"); // 64-bit integer
    tree->Branch("nPulses", &nPulsesCopy, "nPulses/I");   // Integer
    tree->Branch("asicId", &asicIdCopy);                 // TString
    tree->Branch("readDiscList", &discListVec);          // Vector

    // Step 3: Fill the tree with the current object data
    tree->Fill();

    // Step 4: Return the constructed TTree
    return tree;
}


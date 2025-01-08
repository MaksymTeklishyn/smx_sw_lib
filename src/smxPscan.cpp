#include "smxPscan.h"
#include <TFile.h>
#include <TCanvas.h>
#include <TAxis.h>
#include <TNamed.h>
#include <TParameter.h>
#include <TMath.h>
#include <RooRealVar.h>
#include <RooDataSet.h>
#include <RooArgSet.h>
#include <RooPlot.h>
#include <RooCategory.h>
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

    if (std::regex_search(line, match, disc_list_regex)) {
        std::stringstream ss(match[1]);
        int pos;
        readDiscList.clear(); // Clear any existing entries
        while (ss >> pos) {
            readDiscList.push_back(pos);
            if (ss.peek() == ',') ss.ignore();
        }
    }

    // Debugging output to confirm positions
    std::cout << "Parsed DISC_LIST positions: ";
    for (const auto& pos : readDiscList) {
        std::cout << pos << " ";
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
        return pscanTree;
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

            // Reset adc array to zero for each entry
            std::fill(std::begin(adc), std::end(adc), 0);

            // Extract ADC values into a string and parse them into the array
            std::string adc_values_str = data_match[3];
            std::istringstream iss(adc_values_str);
            int value;
            size_t index = 0;
            while (iss >> value) {
                if (index < readDiscList.size() && readDiscList[index] < smxNAdc) {
                    adc[readDiscList[index]] = value;
                } else if (index == readDiscList.size()) {
                    tcomp = value; // Last value as timing comparator
                }
                ++index;
            }

            // Fill the TTree
            pscanTree->Fill();
        } else {
            logError("Failed to match the line: " + line);
        }
    }

    // Close the file and return the TTree
    asciiFile.close();
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
const std::vector<int>& smxPscan::getReadDiscList() const {
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


RooDataSet* smxPscan::toRooDataSet(int channelN) const {
    // Step 1: Define RooRealVars for pulse amplitude, count number, normalized count, and RooCategory for adcComp
    RooRealVar pulseAmp("pulseAmp", "Pulse amplitude", 0, 256, "a.u."); // Range of pulse amplitudes
    RooRealVar countN("countN", "Comparator counts", 0, 300);           // Range of counts
    RooRealVar countNorm("countNorm", "Normalized comparator counts", -2, 3); // Normalized counts
    RooCategory adcComp("adcComp", "ADC Comparator");

    // Define adcComp categories for the comparators in readDiscList
    for (size_t i = 0; i < readDiscList.size(); ++i) {
        int compIndex = readDiscList.at(i);
        adcComp.defineType(Form("Comp%02d", compIndex), compIndex);
        std::cout << compIndex << std::endl;
    }

    // Combine variables into an ArgSet
    RooArgSet variables(pulseAmp, countN, countNorm, adcComp);

    // Step 2: Create a new RooDataSet
    std::cout << "Creating RooDataSet for channel: " << channelN << " and specified comparators in readDiscList." << std::endl;

    RooDataSet* dataset = new RooDataSet("pscanData", "Pulse vs Comparator Data", variables, RooFit::StoreAsymError(variables));

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

    float norm = 1.0 / nPulses;

    std::cout << "Branch addresses set. Looping through TTree entries..." << std::endl;

    float visSepar = 0.02; // Hardcoded control variable

    // Step 5: Loop over TTree entries and filter for the specified channel
    for (Long64_t i = 0; i < pscanTree->GetEntries(); ++i) {
        pscanTree->GetEntry(i);

        // Filter for the specified channel
        if (channel == channelN) {
            pulseAmp.setVal(pulse);

            // Loop over comparators specified in readDiscList
            for (size_t j = 0; j < readDiscList.size(); ++j) {
                int compIndex = readDiscList.at(j);
                if (compIndex == 31) continue; // time comp to be handled separately 
                countN.setVal(adc[compIndex]);
                applyWillsonErrors(&countN);

                countNorm.setVal(countN.getVal() * norm - visSepar * (smxNAdc - 1 - compIndex));
                countNorm.setAsymError(countN.getAsymErrorLo() * norm, countN.getAsymErrorHi() * norm);

                adcComp.setIndex(compIndex); // Set the adcComp value
                dataset->add(variables);
            }
        }
    }

    std::cout << "Finished creating RooDataSet. Total entries: " << dataset->numEntries() << std::endl;


    return dataset;
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
    // Create a new TTree instance
    TTree* tree = new TTree("pscanSettingsTree", "Settings Tree for SMX Pscan");

    Long64_t readTimeLong = static_cast<Long64_t>(readTime);
    int nPulsesCopy = nPulses;
    TString asicIdCopy = asicId;

    // Create a non-const copy of readDiscList to pass to TTree::Branch
    std::vector<int> discListVec = readDiscList;

    // Create branches
    tree->Branch("readTime", &readTimeLong, "readTime/L");
    tree->Branch("nPulses", &nPulsesCopy, "nPulses/I");
    tree->Branch("asicId", &asicIdCopy);
    tree->Branch("readDiscList", &discListVec); // Pass the non-const vector

    // Fill the tree
    tree->Fill();
    return tree;
}

void smxPscan::applyAsymmetricPoissonianErrors(RooRealVar* countN) const {
    if (!countN) {
        std::cerr << "Error: Null pointer passed to applyAsymmetricPoissonianErrors." << std::endl;
        return;
    }
    double count = countN->getVal(); // Proportion of successes
    double lowerError = 0;
    double upperError = 1.841;
    if(count != 0) {
        lowerError = -TMath::Sqrt(count -.25);   // Lower error approximation
        upperError = TMath::Sqrt(count + .75);   // Upper error approximation
    }
    countN->setAsymError(lowerError, upperError);  // Relative to the central value
}

void smxPscan::applyWillsonErrors(RooRealVar* countN) const {
    // Ensure the input pointer is valid
    if (!countN) {
        std::cerr << "Error: Null pointer passed to applyWillsonErrors." << std::endl;
        return;
    }

    int n = nPulses; // Total number of trials (or pulses)
    if (n == 0) {
        std::cerr << "Error: Total number of trials (nPulses) cannot be zero." << std::endl;
        return;
    }

    double p_hat = countN->getVal() / n; // Proportion of successes
    if (p_hat < 0 || p_hat > 1) {
        applyAsymmetricPoissonianErrors(countN); // Handle invalid probabilities
        return;
    }

    double z = 1.0; // z-value for confidence interval
    double z2 = z * z; // Precompute z-squared for efficiency

    // Compute the Wilson score interval with continuity correction
    double sqrtTermMinus = p_hat != 0 
        ? z * sqrt(z2 - 2 - (1.0 / n) + 4 * p_hat * (n * (1 - p_hat) + 1)) 
        : 0.0;
    double sqrtTermPlus = p_hat != 1 
        ? z * sqrt(z2 + 2 - (1.0 / n) + 4 * p_hat * (n * (1 - p_hat) - 1)) 
        : 0.0;

    double w_cc_minus = p_hat != 0 
        ? std::max(0.0, (2 * n * p_hat + z2 - 1 - sqrtTermMinus) / (2 * (n + z2))) 
        : 0.0;
    double w_cc_plus = p_hat < 1 
        ? std::min(1.0, (2 * n * p_hat + z2 + 1 + sqrtTermPlus) / (2 * (n + z2))) 
        : 1.0;

    // Update the RooRealVar object with the calculated asymmetric errors
    countN->setAsymError(n * w_cc_minus - n * p_hat -.5, n * w_cc_plus - n * p_hat +.5);
}





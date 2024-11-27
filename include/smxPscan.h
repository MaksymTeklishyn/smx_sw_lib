#ifndef SMX_PSCAN_H
#define SMX_PSCAN_H

#include "smxConstants.h"
#include <TTree.h>
#include <TString.h>
#include <TArrayI.h>
#include <RooDataSet.h>
#include <fstream>
#include <string>
#include <vector>
#include <ctime>
#include "smxAsicSettings.h"

/**
 * @class smxPscan
 * @brief Class for managing pulse scan data from an ASCII file and converting it into ROOT-compatible formats.
 *
 * This class provides functionality for:
 * - Reading ASCII files containing pulse scan data.
 * - Storing data in a ROOT TTree.
 * - Converting data into a RooDataSet for statistical analysis.
 * - Managing ASIC settings related to the scan.
 */
class smxPscan {
private:
    TTree* pscanTree;                   ///< Internal TTree to store parsed data.
    std::string asciiFileName;          ///< Name of the ASCII file being read.
    std::string asciiFileAddress;       ///< Path to the ASCII file.
    std::vector<int> readDiscList;      ///< Positions of discriminators from the DISC_LIST.

    std::time_t readTime;               ///< Timestamp of the scan (epoch time).
    TString asicId;                     ///< ASIC identifier string (e.g., "XA-000-...").
    int nPulses = 100;                  ///< Number of pulses used in the scan.
    smxAsicSettings asicSettings;       ///< Settings for the ASIC used in the scan.

    /**
     * @brief Creates a TTree representing the settings of the scan.
     * @return A pointer to the generated TTree.
     */
    TTree* settingsToTree() const;

    /**
     * @brief Parses the header line of the ASCII file.
     * @param line The header line from the file.
     */
    void parseHeaderLine(const std::string& line);

    /**
     * @brief Extracts relevant information from the ASCII file name.
     */
    void parseAsciiFileName();

    /**
     * @brief Formats the read time into a human-readable string.
     * @return A formatted string representing the read time.
     */
    std::string formatReadTime() const;

    /**
     * @brief Logs an error message.
     * @param message The message to log.
     */
    void logError(const std::string& message) const;

    /**
     * @brief Generates a default output file name based on the ASCII file name.
     * @return The generated file name.
     */
    std::string generateDefaultOutputFileName() const;

    /**
     * @brief Applies asymmetric Poissonian errors to a RooRealVar.
     * @param countN Pointer to the variable to modify.
     * @param count The count value used to calculate the errors.
     */
    void applyAsymmetricPoissonianErrors(RooRealVar* countN) const;

    /**
     * @brief Applies Wilson score confidence interval errors to a RooRealVar.
     * @param countN Pointer to the variable to modify.
     */
    void applyWillsonErrors(RooRealVar* countN) const;

public:
    /**
     * @brief Default constructor.
     */
    smxPscan();

    /**
     * @brief Destructor to free resources.
     */
    ~smxPscan();

    /**
     * @brief Converts the pulse scan data to a RooDataSet.
     * @param channelN The channel number to include.
     * @param comparator The comparator index.
     * @return A pointer to the generated RooDataSet.
     */
    RooDataSet* toRooDataSet(int channelN) const;

    /**
     * @brief Reads an ASCII file and populates the internal TTree.
     * @param filename The path to the ASCII file.
     * @return A pointer to the populated TTree.
     */
    TTree* readAsciiFile(const std::string& filename);

    /**
     * @brief Writes the TTree to a ROOT file.
     * @param outputFileName The name of the output file (optional).
     */
    void writeRootFile(const std::string& outputFileName = "");

    /**
     * @brief Retrieves the internal TTree.
     * @return A pointer to the TTree.
     */
    TTree* getDataTree() const;

    /**
     * @brief Retrieves the ASCII file name.
     * @return The file name as a string.
     */
    std::string getAsciiFileName() const;

    /**
     * @brief Retrieves the ASCII file address.
     * @return The file path as a string.
     */
    std::string getAsciiFileAddress() const;

    /**
     * @brief Retrieves the discriminator list positions.
     * @return A reference to the vector containing the positions.
     */
    const std::vector<int>& getReadDiscList() const;

    /**
     * @brief Retrieves the read time as epoch time.
     * @return The read time.
     */
    std::time_t getReadTime() const;

    /**
     * @brief Retrieves the ASIC identifier.
     * @return The ASIC ID as a TString.
     */
    TString getAsicId() const;

    /**
     * @brief Retrieves the number of pulses in the scan.
     * @return The number of pulses.
     */
    int getNPulses() const;

    /**
     * @brief Retrieves the ASIC settings.
     * @return A reference to the smxAsicSettings object.
     */
    smxAsicSettings& getAsicSettings();

    /**
     * @brief Updates the ASIC settings.
     * @param settings The new settings to apply.
     */
    void setAsicSettings(const smxAsicSettings& settings);

    /**
     * @brief Displays the entries of the internal TTree in the terminal.
     */
    void showTreeEntries() const;
};

#endif // SMX_PSCAN_H


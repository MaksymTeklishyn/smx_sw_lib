#ifndef SMX_PSCAN_H
#define SMX_PSCAN_H

#include <TTree.h>
#include <TObject.h> // Include TObject for ROOT I/O compatibility
#include <fstream>
#include <string>
#include <vector>
#include <ctime>  // For std::time_t and std::tm

/**
 * @class smxPscan
 * @brief Class for reading pulse scan data from an ASCII file and storing it in a ROOT TTree.
 */
class smxPscan : public TObject {
private:
    TTree* pscanTree;                   ///< Internal field to store the TTree.
    std::string asciiFileName;          ///< Field to store the ASCII file name.
    std::string asciiFileAddress;       ///< Field to store the ASCII file address.
    std::ifstream asciiFile;            ///< Field for the input file stream.
    std::vector<int> readDiscList;      ///< Field to store DISC_LIST positions.

    // Parsed data fields
    std::time_t readTime;               ///< Parsed read time as a numeric value (epoch time).
    std::string asicId;                 ///< Parsed ASIC ID (e.g., "XA-000-08-002-000-002-205-02").
    int nPulses;                        ///< Parsed number of pulses (e.g., 100).

    /**
     * @brief Helper function to parse the first line of the ASCII file and populate readDiscList.
     * @param line The header line to be parsed.
     */
    void parseHeaderLine(const std::string& line);

    /**
     * @brief Helper function to parse the ASCII file name and extract relevant fields (read time, ASIC ID, and number of pulses).
     */
    void parseAsciiFileName();

    /**
     * @brief Helper function to convert readTime to a human-readable string format.
     * @return A string representing the formatted read time.
     */
    std::string formatReadTime() const;

public:
    /**
     * @brief Constructor to initialize the TTree.
     */
    smxPscan();

    /**
     * @brief Destructor to manage memory and close the file if necessary.
     */
    ~smxPscan();

    /**
     * @brief Reads an ASCII file, extracts data, and fills the TTree.
     * @param filename The name of the ASCII file to read.
     * @return A pointer to the filled TTree.
     */
    TTree* readAsciiFile(const std::string& filename);

    /**
     * @brief Writes the TTree to a ROOT file.
     * @param outputFileName The name of the output ROOT file. If empty, a default name based on the input file name is used.
     */
    void writeRootFile(const std::string& outputFileName = "");

    /**
     * @brief Getter to access the internal TTree.
     * @return A pointer to the TTree.
     */
    TTree* getTree() const;

    /**
     * @brief Getter for the ASCII file name.
     * @return The name of the ASCII file.
     */
    std::string getAsciiFileName() const;

    /**
     * @brief Getter for the ASCII file address.
     * @return The address/path of the ASCII file.
     */
    std::string getAsciiFileAddress() const;

    /**
     * @brief Getter for the read DISC_LIST positions.
     * @return A reference to the vector containing DISC_LIST positions.
     */
    const std::vector<int>& getReadDiscList() const;

    /**
     * @brief Getter for the parsed read time as epoch time.
     * @return The read time as a std::time_t value.
     */
    std::time_t getReadTime() const;

    /**
     * @brief Getter for the parsed ASIC ID.
     * @return The ASIC ID as a string.
     */
    std::string getAsicId() const;

    /**
     * @brief Getter for the parsed number of pulses.
     * @return The number of pulses.
     */
    int getNPulses() const;
};

#endif // SMX_PSCAN_H


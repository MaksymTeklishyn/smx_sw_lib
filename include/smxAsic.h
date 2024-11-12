#ifndef SMX_ASIC_H
#define SMX_ASIC_H

#include "smxPscan.h"
#include "smxAsicSettings.h"
#include <vector>
#include <TString.h>

/**
 * @class smxAsic
 * @brief Represents an ASIC with settings and associated pscan data.
 */
class smxAsic {
public:
    smxAsic() = default;  ///< Default constructor

    /**
     * @brief Constructor with explicit ASIC ID and settings.
     * @param id The ASIC ID.
     * @param settings The smxAsicSettings for this ASIC.
     */
    smxAsic(const TString& id, const smxAsicSettings& settings);

    /**
     * @brief Adds a pscan to the ASIC.
     * @param pscan The smxPscan object to add.
     */
    void addPscan(const smxPscan& pscan);

    /**
     * @brief Retrieves the ASIC ID.
     * @return The ASIC ID as a TString.
     */
    TString getAsicId() const;

    /**
     * @brief Retrieves the ASIC settings.
     * @return The ASIC settings as an smxAsicSettings object.
     */
    smxAsicSettings getAsicSettings() const;

    /**
     * @brief Retrieves all pscan data associated with this ASIC.
     * @return A reference to the vector of smxPscan objects.
     */
    const std::vector<smxPscan>& getPscanData() const;

private:
    TString asicId = "XA-000-00-000-000-000-000-00";  ///< Default ASIC ID
    smxAsicSettings asicSettings;  ///< Settings of the ASIC
    std::vector<smxPscan> pscanData;  ///< Vector of pscan data
};

#endif // SMX_ASIC_H


#ifndef SMX_ASIC_SETTINGS_H
#define SMX_ASIC_SETTINGS_H

#include "smxConstants.h"
#include <TTree.h>

/**
 * @class smxAsicSettings
 * @brief Class to store and manage ASIC settings.
 */
class smxAsicSettings {
private:
    int Pol;          ///< Polarity setting
    int Vref_p;       ///< Reference voltage positive
    int Vref_n;       ///< Reference voltage negative
    int Thr2_glb;     ///< Global threshold setting
    int Vref_t;       ///< Reference voltage threshold
    int Vref_t_range; ///< Range for Vref_t setting

public:
    // Constructors
    smxAsicSettings();
    smxAsicSettings(int pol, int vref_p, int vref_n, int thr2_glb, int vref_t, int vref_t_range);

    // Getters
    int getPol() const;
    int getVref_p() const;
    int getVref_n() const;
    int getThr2_glb() const;
    int getVref_t() const;
    int getVref_t_range() const;

    // Setters
    void setPol(int value);
    void setVref_p(int value);
    void setVref_n(int value);
    void setThr2_glb(int value);
    void setVref_t(int value);
    void setVref_t_range(int value);

    /**
     * @brief Creates and returns a TTree with all settings as branches.
     * @param treeName The name of the TTree.
     * @return Pointer to the single-entry TTree containing all settings.
     */
    TTree* toTree(const char* treeName = "AsicSettingsTree") const;
};

#endif // SMX_ASIC_SETTINGS_H


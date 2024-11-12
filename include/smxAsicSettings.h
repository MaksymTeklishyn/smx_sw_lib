#ifndef SMX_ASIC_SETTINGS_H
#define SMX_ASIC_SETTINGS_H

#include "smxConstants.h"

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
    /**
     * @brief Default constructor to initialize default values.
     */
    smxAsicSettings() 
        : Pol(1), Vref_p(58), Vref_n(19), Thr2_glb(34), 
          Vref_t(118), Vref_t_range(1) {}

    /**
     * @brief Constructor to initialize with specific values for all settings.
     * @param pol Polarity setting.
     * @param vref_p Reference voltage positive.
     * @param vref_n Reference voltage negative.
     * @param thr2_glb Global threshold setting.
     * @param vref_t Reference voltage threshold.
     * @param vref_t_range Range for Vref_t setting.
     */
    smxAsicSettings(int pol, int vref_p, int vref_n, int thr2_glb, int vref_t, int vref_t_range) 
        : Pol(pol), Vref_p(vref_p), Vref_n(vref_n), Thr2_glb(thr2_glb), 
          Vref_t(vref_t), Vref_t_range(vref_t_range) {}

    // Getters
    int getPol() const { return Pol; }
    int getVref_p() const { return Vref_p; }
    int getVref_n() const { return Vref_n; }
    int getThr2_glb() const { return Thr2_glb; }
    int getVref_t() const { return Vref_t; }
    int getVref_t_range() const { return Vref_t_range; }

    // Setters
    void setPol(int value) { Pol = value; }
    void setVref_p(int value) { Vref_p = value; }
    void setVref_n(int value) { Vref_n = value; }
    void setThr2_glb(int value) { Thr2_glb = value; }
    void setVref_t(int value) { Vref_t = value; }
    void setVref_t_range(int value) { Vref_t_range = value; }
};

#endif // SMX_ASIC_SETTINGS_H


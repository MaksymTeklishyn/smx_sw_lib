#include "smxAsicSettings.h"
#include <TTree.h>

// Default constructor to initialize default values.
smxAsicSettings::smxAsicSettings()
    : Pol(1), Vref_p(58), Vref_n(19), Thr2_glb(34), Vref_t(118), Vref_t_range(1) {}

// Constructor to initialize with specific values for all settings.
smxAsicSettings::smxAsicSettings(int pol, int vref_p, int vref_n, int thr2_glb, int vref_t, int vref_t_range)
    : Pol(pol), Vref_p(vref_p), Vref_n(vref_n), Thr2_glb(thr2_glb), Vref_t(vref_t), Vref_t_range(vref_t_range) {}

// Getters
int smxAsicSettings::getPol() const { return Pol; }
int smxAsicSettings::getVref_p() const { return Vref_p; }
int smxAsicSettings::getVref_n() const { return Vref_n; }
int smxAsicSettings::getThr2_glb() const { return Thr2_glb; }
int smxAsicSettings::getVref_t() const { return Vref_t; }
int smxAsicSettings::getVref_t_range() const { return Vref_t_range; }

// Setters
void smxAsicSettings::setPol(int value) { Pol = value; }
void smxAsicSettings::setVref_p(int value) { Vref_p = value; }
void smxAsicSettings::setVref_n(int value) { Vref_n = value; }
void smxAsicSettings::setThr2_glb(int value) { Thr2_glb = value; }
void smxAsicSettings::setVref_t(int value) { Vref_t = value; }
void smxAsicSettings::setVref_t_range(int value) { Vref_t_range = value; }

// Method to return all settings as a single-entry TTree
TTree* smxAsicSettings::toTree(const char* treeName) const {
    // Create a new TTree with the specified name
    TTree* tree = new TTree(treeName, "Single-entry TTree for smxAsicSettings");

    // Add branches for each setting
    tree->Branch("Pol", (void*)&Pol, "Pol/I");
    tree->Branch("Vref_p", (void*)&Vref_p, "Vref_p/I");
    tree->Branch("Vref_n", (void*)&Vref_n, "Vref_n/I");
    tree->Branch("Thr2_glb", (void*)&Thr2_glb, "Thr2_glb/I");
    tree->Branch("Vref_t", (void*)&Vref_t, "Vref_t/I");
    tree->Branch("Vref_t_range", (void*)&Vref_t_range, "Vref_t_range/I");

    // Fill the TTree with the current settings
    tree->Fill();

    return tree;
}


#include "smxAsic.h"
#include <iostream>

smxAsic::smxAsic(const TString& id, const smxAsicSettings& settings)
    : asicId(id), asicSettings(settings) {}

void smxAsic::addPscan(const smxPscan& pscan) {
    // Check if current asicId is null, empty, or the default placeholder
    if (asicId.IsNull() || asicId == "" || asicId == "XA-000-00-000-000-000-000-00") {
        asicId = pscan.getAsicId();  // Set asicId from pscan
        pscanData.push_back(pscan);
    } else if (asicId == pscan.getAsicId()) {
        // IDs match, add the pscan
        pscanData.push_back(pscan);
    } else {
        // IDs do not match, log error and do not add
        std::cerr << "Error: Attempt to add pscan with different ASIC ID ("
                  << pscan.getAsicId() << ") to ASIC with ID " << asicId << "." << std::endl;
    }
}

void smxAsic::setAsicId(const TString& id) {
    asicId = id;
}

TString smxAsic::getAsicId() const {
    return asicId;
}

smxAsicSettings smxAsic::getAsicSettings() const {
    return asicSettings;
}

const std::vector<smxPscan>& smxAsic::getPscanData() const {
    return pscanData;
}


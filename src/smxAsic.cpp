#include "smxAsic.h"
#include <iostream>

smxAsic::smxAsic(const TString& id, const smxAsicSettings& settings)
    : asicId(id), asicSettings(settings) {}

void smxAsic::addPscan(smxPscan* pscan) {
    if (!pscan) {
        std::cerr << "Error: nullptr passed to addPscan." << std::endl;
        return;
    }

    if (asicId.IsNull() || asicId == "XA-000-00-000-000-000-000-00") {
        asicId = pscan->getAsicId();
    } else if (asicId != pscan->getAsicId()) {
        std::cerr << "Error: Mismatched ASIC IDs. Cannot add pscan." << std::endl;
        return;
    }

    pscanData.push_back(*pscan);  // Store the pointer in the collection
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


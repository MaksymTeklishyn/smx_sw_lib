#include "smxAsic.h"

smxAsic::smxAsic(const TString& id, const smxAsicSettings& settings)
    : asicId(id), asicSettings(settings) {}

void smxAsic::addPscan(const smxPscan& pscan) {
    pscanData.push_back(pscan);  // Directly store the smxPscan object
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


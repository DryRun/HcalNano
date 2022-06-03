#include "HCALPFG/HcalNano/interface/HFPreRecHitTable.h"

hcalnano::HFPreRecHitTable::HFPreRecHitTable(std::vector<HcalDetId>& _dids) {
    dids_ = _dids;
    for (std::vector<HcalDetId>::const_iterator it_did = dids_.begin(); it_did != dids_.end(); ++it_did) {
        did_indexmap_[*it_did] = (unsigned int)(it_did - dids_.begin());
    }

    charges_.resize(dids_.size());
    chargeAsymmetries_.resize(dids_.size());
    valids_.resize(dids_.size());
}

void hcalnano::HFPreRecHitTable::add(const HFPreRecHitCollection::const_iterator itPreRecHit) {
    HcalDetId did = itPreRecHit->id();
    unsigned int index = did_indexmap_.at(did);//std::find(dids_.begin(), dids_.end(), did) - dids_.begin();
    //if (index == dids_.size()) {
    //    std::cerr << "[HFPreRecHitTable] ERROR : Didn't find did " << did << " in table" << std::endl;
    //    exit(1);
    //}
    
    charges_[index] = itPreRecHit->charge();
    chargeAsymmetries_[index] = itPreRecHit->chargeAsymmetry(0.).first; // Return std::pair<float qAsym, bool passCut>
    valids_[index] = true;
}

void hcalnano::HFPreRecHitTable::reset() {
    std::fill(charges_.begin(), charges_.end(), 0);
    std::fill(chargeAsymmetries_.begin(), chargeAsymmetries_.end(), 0);
    std::fill(valids_.begin(), valids_.end(), false);
}

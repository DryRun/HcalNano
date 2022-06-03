#ifndef HcalRecHitTable_h
#define HcalRecHitTable_h

#include <vector>
#include <map>

#include "CalibFormats/HcalObjects/interface/HcalCoderDb.h"
#include "CalibFormats/HcalObjects/interface/HcalDbRecord.h"
#include "CalibFormats/HcalObjects/interface/HcalDbService.h"
#include "DataFormats/HcalDetId/interface/HcalSubdetector.h"
#include "DataFormats/HcalDetId/interface/HcalDetId.h"
#include "DataFormats/HcalDetId/interface/HcalGenericDetId.h"
#include "DataFormats/HcalDigi/interface/HcalDigiCollections.h"
#include "CondFormats/HcalObjects/interface/HcalElectronicsMap.h"
#include "CondFormats/HcalObjects/interface/HcalChannelQuality.h"
#include "DataFormats/HcalRecHit/interface/HcalRecHitCollections.h"
#include "DQM/HcalCommon/interface/Utilities.h"

namespace hcalnano {
    template <class RecHitCollection> 
    class HcalRecHitTable {
    public:
        std::vector<HcalDetId> dids_;
        std::map<HcalDetId, unsigned int> did_indexmap_; // Use std::map for efficient lookup, rather than std::find

        std::vector<int> ietas_;
        std::vector<int> iphis_;
        std::vector<int> depths_;
        std::vector<int> rawIds_;
        std::vector<float> energies_;
        std::vector<float> times_;
        std::vector<int> flags_;
        std::vector<int> auxs_;

        std::vector<bool> valids_;

        HcalRecHitTable(std::vector<HcalDetId>& _dids);
        void add(typename RecHitCollection::const_iterator& rechit);
        void reset();

    };

    template <class RecHitCollection>
    HcalRecHitTable<RecHitCollection>::HcalRecHitTable(std::vector<HcalDetId>& _dids) {
        dids_ = _dids;
        for (std::vector<HcalDetId>::const_iterator it_did = dids_.begin(); it_did != dids_.end(); ++it_did) {
            did_indexmap_[*it_did] = (unsigned int)(it_did - dids_.begin());
        }

        ietas_.resize(dids_.size());
        iphis_.resize(dids_.size());
        depths_.resize(dids_.size());
        rawIds_.resize(dids_.size());
        energies_.resize(dids_.size());
        times_.resize(dids_.size());
        flags_.resize(dids_.size());
        auxs_.resize(dids_.size());
        valids_.resize(dids_.size());
    }

    template <class RecHitCollection>
    void HcalRecHitTable<RecHitCollection>::add(typename RecHitCollection::const_iterator& rechit) {
        HcalDetId did = HcalDetId(rechit->detid());
        unsigned int index = did_indexmap_.at(did);

        ietas_[index]  = did.ieta();
        iphis_[index]  = did.iphi();
        depths_[index] = did.depth();
        rawIds_[index] = did.rawId();

        energies_[index] = rechit->energy();
        times_[index]    = rechit->time();
        flags_[index]    = rechit->flags();
        auxs_[index]     = rechit->aux();

        valids_[index] = true;
    };

    template <class RecHitCollection>
    void HcalRecHitTable<RecHitCollection>::reset() {
        std::fill(ietas_.begin(), ietas_.end(), 0);
        std::fill(iphis_.begin(), iphis_.end(), 0);
        std::fill(depths_.begin(), depths_.end(), 0);
        std::fill(rawIds_.begin(), rawIds_.end(), 0);
        std::fill(energies_.begin(), energies_.end(), 0);
        std::fill(times_.begin(), times_.end(), 0);
        std::fill(flags_.begin(), flags_.end(), 0);
        std::fill(auxs_.begin(), auxs_.end(), 0);
        std::fill(valids_.begin(), valids_.end(), false);
    };

    typedef HcalRecHitTable<HBHERecHitCollection> HBRecHitTable;
    typedef HcalRecHitTable<HBHERecHitCollection> HERecHitTable;
    typedef HcalRecHitTable<HFRecHitCollection> HFRecHitTable;
    typedef HcalRecHitTable<HORecHitCollection> HORecHitTable;
}
#endif

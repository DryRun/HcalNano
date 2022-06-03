// system include files
#include <memory>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"

#include "FWCore/Utilities/interface/ESInputTag.h"
#include "FWCore/Utilities/interface/Transition.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"

#include "DataFormats/NanoAOD/interface/FlatTable.h"

#include "CommonTools/Utils/interface/StringCutObjectSelector.h"
#include "CommonTools/Utils/interface/StringObjectFunction.h"

#include "CalibFormats/HcalObjects/interface/HcalCoderDb.h"
#include "CalibFormats/HcalObjects/interface/HcalDbRecord.h"
#include "CalibFormats/HcalObjects/interface/HcalDbService.h"
#include "DataFormats/HcalDetId/interface/HcalSubdetector.h"
#include "DataFormats/HcalDetId/interface/HcalDetId.h"
#include "DataFormats/HcalDetId/interface/HcalGenericDetId.h"
#include "CondFormats/HcalObjects/interface/HcalElectronicsMap.h"
#include "CondFormats/HcalObjects/interface/HcalChannelQuality.h"
#include "DataFormats/HcalRecHit/interface/HcalRecHitCollections.h"
#include "DQM/HcalCommon/interface/Utilities.h"


#include "HCALPFG/HcalNano/interface/HcalRecHitTable.h"
#include "HCALPFG/HcalNano/interface/HFPreRecHitTable.h"
#include "DataFormats/HcalNano/interface/HcalChannelInfo.h"

namespace hcalnano {
    class HcalRecHitTableProducer : public edm::stream::EDProducer<> {
    private:
        std::map<HcalSubdetector, std::vector<HcalDetId> > dids_;
        //std::map<HcalSubdetector, std::vector<HcalElectronicsId> > eids_;
        static const std::vector<HcalSubdetector> subdets_;
        HcalElectronicsMap const *emap_;

        edm::InputTag tagChannelInfo_;
        edm::EDGetTokenT<hcalnano::HcalChannelInfo> tokenChannelInfo_;

        edm::ESGetToken<HcalDbService, HcalDbRecord> tokenHcalDbService_;
        edm::ESHandle<HcalDbService> dbService_;

        edm::InputTag tagHBHE_;
        edm::EDGetTokenT<HBHERecHitCollection> tokenHBHE_;

        edm::InputTag tagHF_;
        edm::EDGetTokenT<HFRecHitCollection> tokenHF_;

        edm::InputTag tagHO_;
        edm::EDGetTokenT<HORecHitCollection> tokenHO_;

        bool hfPreRecHitsAvailable_;
        edm::InputTag tagPreHF_;
        edm::EDGetTokenT<HFPreRecHitCollection> tokenPreHF_;

        hcalnano::HBRecHitTable *hbRecHitTable_;
        hcalnano::HERecHitTable *heRecHitTable_;
        hcalnano::HFRecHitTable *hfRecHitTable_;
        hcalnano::HORecHitTable *hoRecHitTable_;
        hcalnano::HFPreRecHitTable *hfPreRecHitTable_;

    public:
      explicit HcalRecHitTableProducer(const edm::ParameterSet& iConfig) : 
        tokenChannelInfo_(consumes<edm::InRun>(iConfig.getUntrackedParameter<edm::InputTag>("tagChannelInfo", edm::InputTag("hcalChannelInfoTable")))),
        tokenHcalDbService_(esConsumes<HcalDbService, HcalDbRecord, edm::Transition::BeginRun>()),
        tagHBHE_(iConfig.getUntrackedParameter<edm::InputTag>("tagHBHE", edm::InputTag("hbhereco"))), 
        tagHF_(iConfig.getUntrackedParameter<edm::InputTag>("tagHF", edm::InputTag("hfreco"))), 
        tagHO_(iConfig.getUntrackedParameter<edm::InputTag>("tagHO", edm::InputTag("horeco"))),
        hfPreRecHitsAvailable_(iConfig.getUntrackedParameter<bool>("hfPreRecHitsAvailable", false)), 
        tagPreHF_(iConfig.getUntrackedParameter<edm::InputTag>("tagPreHF", edm::InputTag("")))
        {

        //tokenChannelInfo_ = consumes<hcalnano::HcalChannelInfo, edm::InRun>(tagChannelInfo_);

        tokenHBHE_ = consumes<HBHERecHitCollection>(tagHBHE_);
        tokenHF_ = consumes<HFRecHitCollection>(tagHF_);
        tokenPreHF_ = consumes<HFPreRecHitCollection>(tagPreHF_);
        tokenHO_ = consumes<HORecHitCollection>(tagHO_);

        produces<nanoaod::FlatTable>("HBRecHitTable");
        produces<nanoaod::FlatTable>("HERecHitTable");
        produces<nanoaod::FlatTable>("HFRecHitTable");
        produces<nanoaod::FlatTable>("HORecHitTable");

        //tagQIE11_ = iConfig.getUntrackedParameter<edm::InputTag>("tagQIE11", edm::InputTag("hcalRecHits"));
        //tagHO_ = iConfig.getUntrackedParameter<edm::InputTag>("tagHO", edm::InputTag("hcalRecHits"));
        //tagQIE10_ = iConfig.getUntrackedParameter<edm::InputTag>("tagHF", edm::InputTag("hcalRecHits"));

        hbRecHitTable_ = 0;
        heRecHitTable_ = 0;
        hfRecHitTable_ = 0;
        hoRecHitTable_ = 0;
        hfPreRecHitTable_ = 0;

      }

        ~HcalRecHitTableProducer() {
            delete hbRecHitTable_;
            delete heRecHitTable_;
            delete hfRecHitTable_;
            delete hoRecHitTable_;
            if (hfPreRecHitTable_) {
                delete hfPreRecHitTable_;
            }
        };

        /*
        static void fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
            edm::ParameterSetDescription desc;
            desc.add<edm::InputTag>("tagQIE11")->setComment("Input QIE 11 digi collection");
            // desc.add<std::string>("name")->setComment("");
            descriptions.add("HcalRecHitTable", desc);
        }
        */

    private:
        void beginRun(edm::Run const&, edm::EventSetup const&);
        void produce(edm::Event&, edm::EventSetup const&) override;

    };
}

const std::vector<HcalSubdetector> hcalnano::HcalRecHitTableProducer::subdets_ = {HcalBarrel, HcalEndcap, HcalForward, HcalOuter};

void hcalnano::HcalRecHitTableProducer::beginRun(edm::Run const& iRun, edm::EventSetup const& iSetup) {
    // List DetIds of interest from emap
    dbService_ = iSetup.getHandle(tokenHcalDbService_);
    emap_ = dbService_->getHcalMapping();

    //std::vector<HcalGenericDetId> dids = emap_->allPrecisionId();
    edm::Handle<hcalnano::HcalChannelInfo> channelInfo;
    iRun.getByToken(tokenChannelInfo_, channelInfo);

    dids_ = channelInfo->dids;
    //eids_ = channelInfo->eids;

    // Create persistent digi storage
    hbRecHitTable_ = new hcalnano::HBRecHitTable(dids_[HcalBarrel]);
    heRecHitTable_ = new hcalnano::HERecHitTable(dids_[HcalEndcap]);
    hfRecHitTable_ = new hcalnano::HFRecHitTable(dids_[HcalForward]);
    hoRecHitTable_ = new hcalnano::HORecHitTable(dids_[HcalOuter]);
    if (hfPreRecHitsAvailable_) {
        hfPreRecHitTable_ = new hcalnano::HFPreRecHitTable(dids_[HcalForward]);
    }
}


void hcalnano::HcalRecHitTableProducer::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {
    // * Load digis */
    edm::Handle<HBHERecHitCollection> hbheRecHits;
    iEvent.getByToken(tokenHBHE_, hbheRecHits);

    edm::Handle<HFRecHitCollection> hfRecHits;
    iEvent.getByToken(tokenHF_, hfRecHits);

    edm::Handle<HORecHitCollection> hoRecHits;
    iEvent.getByToken(tokenHO_, hoRecHits);

    // * Process digis */
    // HB and HE together
    hbRecHitTable_->reset();
    heRecHitTable_->reset();
    for (HBHERecHitCollection::const_iterator itRecHit = hbheRecHits->begin(); itRecHit != hbheRecHits->end(); ++itRecHit) {
        HcalDetId const& did = itRecHit->id();
        if (did.subdet() == HcalBarrel) {
            hbRecHitTable_->add(itRecHit);
        } else if (did.subdet() == HcalEndcap) {
            heRecHitTable_->add(itRecHit);
        }
    } // End loop over HBHE RecHits

    // HF
    hfRecHitTable_->reset();
    for (HFRecHitCollection::const_iterator itRecHit = hfRecHits->begin(); itRecHit != hfRecHits->end(); ++itRecHit) {
        HcalDetId const& did = itRecHit->id();
        if (did.subdet() != HcalForward) continue;

        hfRecHitTable_->add(itRecHit);
    } // End loop over HF RecHits


    // HO
    hoRecHitTable_->reset();
    for (HORecHitCollection::const_iterator itRecHit = hoRecHits->begin(); itRecHit != hoRecHits->end(); ++itRecHit) {
        HcalDetId const& did = itRecHit->id();
        if (did.subdet() != HcalOuter) continue;

        hoRecHitTable_->add(itRecHit);
    } // End loop over HO digis

    // HF pre
    if (hfPreRecHitsAvailable_) {
        edm::Handle<HFPreRecHitCollection> hfPreRecHits;
        iEvent.getByToken(tokenPreHF_, hfPreRecHits);

        hfPreRecHitTable_->reset();
        for (HFPreRecHitCollection::const_iterator itPreRecHit = hfPreRecHits->begin(); itPreRecHit != hfPreRecHits->end(); ++itPreRecHit) {
            HcalDetId const& did = itPreRecHit->id();
            if (did.subdet() != HcalForward) continue;

            hfPreRecHitTable_->add(itPreRecHit);
        }
    }

    // * Save to NanoAOD tables */

    // HB
    auto hbNanoTable = std::make_unique<nanoaod::FlatTable>(dids_[HcalBarrel].size(), "HBRecHits", false, false);
    hbNanoTable->addColumn<int>("ieta", hbRecHitTable_->ietas_, "ieta");
    hbNanoTable->addColumn<int>("iphi", hbRecHitTable_->iphis_, "iphi");
    hbNanoTable->addColumn<int>("depth", hbRecHitTable_->depths_, "depth");
    hbNanoTable->addColumn<int>("rawId", hbRecHitTable_->rawIds_, "rawId");
    hbNanoTable->addColumn<float>("energy", hbRecHitTable_->energies_, "energy");
    hbNanoTable->addColumn<float>("time", hbRecHitTable_->times_, "time");
    hbNanoTable->addColumn<int>("flags", hbRecHitTable_->flags_, "flags");
    hbNanoTable->addColumn<int>("aux", hbRecHitTable_->auxs_, "aux");
    hbNanoTable->addColumn<bool>("valid", hbRecHitTable_->valids_, "valid");
    iEvent.put(std::move(hbNanoTable), "HBRecHitTable");

    // HE
    auto heNanoTable = std::make_unique<nanoaod::FlatTable>(dids_[HcalEndcap].size(), "HERecHits", false, false);
    heNanoTable->addColumn<int>("ieta", heRecHitTable_->ietas_, "ieta");
    heNanoTable->addColumn<int>("iphi", heRecHitTable_->iphis_, "iphi");
    heNanoTable->addColumn<int>("depth", heRecHitTable_->depths_, "depth");
    heNanoTable->addColumn<int>("rawId", heRecHitTable_->rawIds_, "rawId");
    heNanoTable->addColumn<float>("energy", heRecHitTable_->energies_, "energy");
    heNanoTable->addColumn<float>("time", heRecHitTable_->times_, "time");
    heNanoTable->addColumn<int>("flags", heRecHitTable_->flags_, "flags");
    heNanoTable->addColumn<int>("aux", heRecHitTable_->auxs_, "aux");
    heNanoTable->addColumn<bool>("valid", heRecHitTable_->valids_, "valid");
    iEvent.put(std::move(heNanoTable), "HERecHitTable");

    // HF
    auto hfNanoTable = std::make_unique<nanoaod::FlatTable>(dids_[HcalForward].size(), "HFRecHits", false, false);
    hfNanoTable->addColumn<int>("ieta", hfRecHitTable_->ietas_, "ieta");
    hfNanoTable->addColumn<int>("iphi", hfRecHitTable_->iphis_, "iphi");
    hfNanoTable->addColumn<int>("depth", hfRecHitTable_->depths_, "depth");
    hfNanoTable->addColumn<int>("rawId", hfRecHitTable_->rawIds_, "rawId");
    hfNanoTable->addColumn<float>("energy", hfRecHitTable_->energies_, "energy");
    hfNanoTable->addColumn<float>("time", hfRecHitTable_->times_, "time");
    hfNanoTable->addColumn<int>("flags", hfRecHitTable_->flags_, "flags");
    hfNanoTable->addColumn<int>("aux", hfRecHitTable_->auxs_, "aux");
    hfNanoTable->addColumn<bool>("valid", hfRecHitTable_->valids_, "valid");
    if (hfPreRecHitsAvailable_) {
        hfNanoTable->addColumn<float>("charge", hfPreRecHitTable_->charges_, "charge");
        hfNanoTable->addColumn<float>("chargeAsymmetry", hfPreRecHitTable_->chargeAsymmetries_, "chargeAsymmetry");
    }
    iEvent.put(std::move(hfNanoTable), "HFRecHitTable");

    // HO
    auto hoNanoTable = std::make_unique<nanoaod::FlatTable>(dids_[HcalOuter].size(), "HORecHits", false, false);
    hoNanoTable->addColumn<int>("ieta", hoRecHitTable_->ietas_, "ieta");
    hoNanoTable->addColumn<int>("iphi", hoRecHitTable_->iphis_, "iphi");
    hoNanoTable->addColumn<int>("depth", hoRecHitTable_->depths_, "depth");
    hoNanoTable->addColumn<int>("rawId", hoRecHitTable_->rawIds_, "rawId");
    hoNanoTable->addColumn<float>("energy", hoRecHitTable_->energies_, "energy");
    hoNanoTable->addColumn<float>("time", hoRecHitTable_->times_, "time");
    hoNanoTable->addColumn<int>("flags", hoRecHitTable_->flags_, "flags");
    hoNanoTable->addColumn<int>("aux", hoRecHitTable_->auxs_, "aux");
    hoNanoTable->addColumn<bool>("valid", hoRecHitTable_->valids_, "valid");
    iEvent.put(std::move(hoNanoTable), "HORecHitTable");

}

#include "FWCore/Framework/interface/MakerMacros.h"
//define this as a plug-in
DEFINE_FWK_MODULE(hcalnano::HcalRecHitTableProducer);

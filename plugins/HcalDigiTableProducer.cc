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
#include "DQM/HcalCommon/interface/Utilities.h"


#include "HCALPFG/HcalNano/interface/QIE11DigiTable.h"
#include "HCALPFG/HcalNano/interface/QIE10DigiTable.h"
#include "HCALPFG/HcalNano/interface/HODigiTable.h"

class HcalDigiTableProducer : public edm::stream::EDProducer<> {
private:
    std::map<HcalSubdetector, std::vector<HcalDetId> > dids_;
    std::map<HcalSubdetector, std::vector<HcalElectronicsId> > eids_;
    static const std::vector<HcalSubdetector> subdets_;
    HcalElectronicsMap const *emap_;
    std::map<HcalSubdetector, unsigned int> nDigis_;

    edm::InputTag tagQIE11_;
    edm::EDGetTokenT<QIE11DigiCollection> tokenQIE11_;

    edm::InputTag tagQIE10_;
    edm::EDGetTokenT<QIE10DigiCollection> tokenQIE10_;

    edm::InputTag tagHO_;
    edm::EDGetTokenT<HODigiCollection> tokenHO_;

    edm::ESGetToken<HcalDbService, HcalDbRecord> tokenHcalDbService_;
    edm::ESHandle<HcalDbService> dbService_;

    HBDigiTable *hbDigiTable;
    HEDigiTable *heDigiTable;
    HFDigiTable *hfDigiTable;
    HODigiTable *hoDigiTable;

public:
  explicit HcalDigiTableProducer(const edm::ParameterSet& iConfig) : 
    tagQIE11_(iConfig.getUntrackedParameter<edm::InputTag>("tagQIE11", edm::InputTag("hcalDigis"))), 
    tagQIE10_(iConfig.getUntrackedParameter<edm::InputTag>("tagQIE10", edm::InputTag("hcalDigis"))), 
    tagHO_(iConfig.getUntrackedParameter<edm::InputTag>("tagHO", edm::InputTag("hcalDigis"))), 
    tokenHcalDbService_(esConsumes<HcalDbService, HcalDbRecord, edm::Transition::BeginRun>())
    {
    produces<nanoaod::FlatTable>("HB");
    produces<nanoaod::FlatTable>("HE");
    produces<nanoaod::FlatTable>("HF");
    produces<nanoaod::FlatTable>("HO");

    //tagQIE11_ = iConfig.getUntrackedParameter<edm::InputTag>("tagQIE11", edm::InputTag("hcalDigis"));
    //tagHO_ = iConfig.getUntrackedParameter<edm::InputTag>("tagHO", edm::InputTag("hcalDigis"));
    //tagQIE10_ = iConfig.getUntrackedParameter<edm::InputTag>("tagHF", edm::InputTag("hcalDigis"));

    tokenQIE11_ = consumes<QIE11DigiCollection>(tagQIE11_);
    tokenHO_ = consumes<HODigiCollection>(tagHO_);
    tokenQIE10_ = consumes<QIE10DigiCollection>(tagQIE10_);

  }

    ~HcalDigiTableProducer() {
        delete hbDigiTable;
        delete heDigiTable;
        delete hfDigiTable;
        delete hoDigiTable;
    };

    /*
    static void fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
        edm::ParameterSetDescription desc;
        desc.add<edm::InputTag>("tagQIE11")->setComment("Input QIE 11 digi collection");
        // desc.add<std::string>("name")->setComment("");
        descriptions.add("HcalDigiTable", desc);
    }
    */

private:
    void beginRun(edm::Run const&, edm::EventSetup const&);
    void produce(edm::Event&, edm::EventSetup const&) override;

};

const std::vector<HcalSubdetector> HcalDigiTableProducer::subdets_ = {HcalBarrel, HcalEndcap, HcalForward, HcalOuter};

void HcalDigiTableProducer::beginRun(edm::Run const& iRun, edm::EventSetup const& iSetup) {
    // List DetIds of interest from emap
    dbService_ = iSetup.getHandle(tokenHcalDbService_);
    emap_ = dbService_->getHcalMapping();

    std::vector<HcalGenericDetId> dids = emap_->allPrecisionId();
    nDigis_[HcalBarrel] = 0;
    nDigis_[HcalEndcap] = 0;
    nDigis_[HcalForward] = 0;
    nDigis_[HcalOuter] = 0;
    for (auto it_did = dids.begin(); it_did != dids.end(); ++it_did) {
        if (!it_did->isHcalDetId()) {
            continue;
        }
        HcalDetId did = HcalDetId(it_did->rawId());
        if (!(did.subdet() == HcalBarrel || did.subdet() == HcalEndcap || did.subdet() == HcalForward || did.subdet() == HcalOuter)) {
            continue;
        }
        dids_[did.subdet()].push_back(did);
        ++nDigis_[did.subdet()];
    }

    // Sort HcalDetIds
    for (auto& it_subdet : subdets_) {
        std::sort(dids_[it_subdet].begin(), dids_[it_subdet].end());
    }

    // Save EIDs
    for (auto& it_subdet : subdets_) {
        for (auto it_did : dids_[it_subdet]) {
            eids_[it_subdet].push_back(HcalElectronicsId(it_did.rawId()));
        }
    }

    // Create persistent digi storage
    hbDigiTable = new HBDigiTable(dids_[HcalBarrel], 8);
    heDigiTable = new HEDigiTable(dids_[HcalEndcap], 8);
    hfDigiTable = new HFDigiTable(dids_[HcalForward], 3);
    hoDigiTable = new HODigiTable(dids_[HcalOuter], 10);
}


void HcalDigiTableProducer::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {
    // * Load digis */
    edm::Handle<QIE11DigiCollection> qie11Digis;
    iEvent.getByToken(tokenQIE11_, qie11Digis);

    edm::Handle<QIE10DigiCollection> qie10Digis;
    iEvent.getByToken(tokenQIE10_, qie10Digis);

    edm::Handle<HODigiCollection> hoDigis;
    iEvent.getByToken(tokenHO_, hoDigis);

    // * Process digis */
    // HB
    hbDigiTable->reset();
    for (QIE11DigiCollection::const_iterator itDigi = qie11Digis->begin(); itDigi != qie11Digis->end(); ++itDigi) {
        const QIE11DataFrame digi = static_cast<const QIE11DataFrame>(*itDigi);
        HcalDetId const& did = digi.detid();
        if (did.subdet() != HcalBarrel) continue;

        hbDigiTable->add(&digi, dbService_);
    } // End loop over qie11 HB digis


    // HE
    heDigiTable->reset();
    for (QIE11DigiCollection::const_iterator itDigi = qie11Digis->begin(); itDigi != qie11Digis->end(); ++itDigi) {
        const QIE11DataFrame digi = static_cast<const QIE11DataFrame>(*itDigi);
        HcalDetId const& did = digi.detid();
        if (did.subdet() != HcalEndcap) continue;

        heDigiTable->add(&digi, dbService_);
    } // End loop over qie11 HE digis


    // HF
    hfDigiTable->reset();
    for (QIE10DigiCollection::const_iterator itDigi = qie10Digis->begin(); itDigi != qie10Digis->end(); ++itDigi) {
        const QIE10DataFrame digi = static_cast<const QIE10DataFrame>(*itDigi);
        HcalDetId const& did = digi.detid();
        if (did.subdet() != HcalForward) continue;

        hfDigiTable->add(&digi, dbService_);
    } // End loop over qie10 HF digis


    // HO
    hoDigiTable->reset();
    for (HODigiCollection::const_iterator itDigi = hoDigis->begin(); itDigi != hoDigis->end(); ++itDigi) {
        const HODataFrame digi = static_cast<const HODataFrame>(*itDigi);
        HcalDetId const& did = digi.id();
        if (did.subdet() != HcalOuter) continue;

        hoDigiTable->add(&digi, dbService_);
    } // End loop over HO digis


    // * Save to NanoAOD tables */

    // HB
    auto hbNanoTable = std::make_unique<nanoaod::FlatTable>(nDigis_[HcalBarrel], "HBDigis", false, false);
    hbNanoTable->addColumn<int>("rawId", hbDigiTable->rawIds_, "rawId");
    hbNanoTable->addColumn<int>("ieta", hbDigiTable->ietas_, "ieta");
    hbNanoTable->addColumn<int>("iphi", hbDigiTable->iphis_, "iphi");
    hbNanoTable->addColumn<int>("depth", hbDigiTable->depths_, "depth");
    hbNanoTable->addColumn<int>("subdet", hbDigiTable->subdets_, "subdet");
    hbNanoTable->addColumn<bool>("linkError", hbDigiTable->linkErrors_, "linkError");
    hbNanoTable->addColumn<bool>("capidError", hbDigiTable->capidErrors_, "capidError");
    hbNanoTable->addColumn<int>("flags", hbDigiTable->flags_, "flags");
    hbNanoTable->addColumn<int>("soi", hbDigiTable->sois_, "soi");
    hbNanoTable->addColumn<bool>("valid", hbDigiTable->valids_, "valid");

    for (unsigned int iTS = 0; iTS < 8; ++iTS) {
        hbNanoTable->addColumn<int>(std::string("adc") + std::to_string(iTS), 
                                        hbDigiTable->adcs_[iTS], 
                                        std::string("adc") + std::to_string(iTS));
        hbNanoTable->addColumn<int>(std::string("tdc") + std::to_string(iTS), 
                                        hbDigiTable->tdcs_[iTS], 
                                        std::string("tdc") + std::to_string(iTS));
        hbNanoTable->addColumn<int>(std::string("capid") + std::to_string(iTS), 
                                        hbDigiTable->capids_[iTS], 
                                        std::string("capid") + std::to_string(iTS));
        hbNanoTable->addColumn<float>(std::string("fc") + std::to_string(iTS), 
                                        hbDigiTable->fcs_[iTS], 
                                        std::string("fc") + std::to_string(iTS));
        hbNanoTable->addColumn<float>(std::string("pedestalfc") + std::to_string(iTS), 
                                        hbDigiTable->pedestalfcs_[iTS], 
                                        std::string("pedestalfc") + std::to_string(iTS));
    }
    iEvent.put(std::move(hbNanoTable));

    // HE
    auto heNanoTable = std::make_unique<nanoaod::FlatTable>(nDigis_[HcalEndcap], "HEDigis", false, false);
    heNanoTable->addColumn<int>("rawId", heDigiTable->rawIds_, "rawId");
    heNanoTable->addColumn<int>("ieta", heDigiTable->ietas_, "ieta");
    heNanoTable->addColumn<int>("iphi", heDigiTable->iphis_, "iphi");
    heNanoTable->addColumn<int>("depth", heDigiTable->depths_, "depth");
    heNanoTable->addColumn<int>("subdet", heDigiTable->subdets_, "subdet");
    heNanoTable->addColumn<bool>("linkError", heDigiTable->linkErrors_, "linkError");
    heNanoTable->addColumn<bool>("capidError", heDigiTable->capidErrors_, "capidError");
    heNanoTable->addColumn<int>("flags", heDigiTable->flags_, "flags");
    heNanoTable->addColumn<int>("soi", heDigiTable->sois_, "soi");
    heNanoTable->addColumn<bool>("valid", heDigiTable->valids_, "valid");

    for (unsigned int iTS = 0; iTS < 8; ++iTS) {
        heNanoTable->addColumn<int>(std::string("adc") + std::to_string(iTS), 
                                        heDigiTable->adcs_[iTS], 
                                        std::string("adc") + std::to_string(iTS));
        heNanoTable->addColumn<int>(std::string("tdc") + std::to_string(iTS), 
                                        heDigiTable->tdcs_[iTS], 
                                        std::string("tdc") + std::to_string(iTS));
        heNanoTable->addColumn<int>(std::string("capid") + std::to_string(iTS), 
                                        heDigiTable->capids_[iTS], 
                                        std::string("capid") + std::to_string(iTS));
        heNanoTable->addColumn<float>(std::string("fc") + std::to_string(iTS), 
                                        heDigiTable->fcs_[iTS], 
                                        std::string("fc") + std::to_string(iTS));
        heNanoTable->addColumn<float>(std::string("pedestalfc") + std::to_string(iTS), 
                                        heDigiTable->pedestalfcs_[iTS], 
                                        std::string("pedestalfc") + std::to_string(iTS));
    }
    iEvent.put(std::move(heNanoTable));

    // HF
    auto hfNanoTable = std::make_unique<nanoaod::FlatTable>(nDigis_[HcalForward], "HFDigis", false, false);
    hfNanoTable->addColumn<int>("rawId", hfDigiTable->rawIds_, "rawId");
    hfNanoTable->addColumn<int>("ieta", hfDigiTable->ietas_, "ieta");
    hfNanoTable->addColumn<int>("iphi", hfDigiTable->iphis_, "iphi");
    hfNanoTable->addColumn<int>("depth", hfDigiTable->depths_, "depth");
    hfNanoTable->addColumn<int>("subdet", hfDigiTable->subdets_, "subdet");
    hfNanoTable->addColumn<bool>("linkError", hfDigiTable->linkErrors_, "linkError");
    hfNanoTable->addColumn<int>("flags", hfDigiTable->flags_, "flags");
    hfNanoTable->addColumn<int>("soi", hfDigiTable->sois_, "soi");
    hfNanoTable->addColumn<bool>("valid", hfDigiTable->valids_, "valid");

    for (unsigned int iTS = 0; iTS < 3; ++iTS) {
        hfNanoTable->addColumn<int>(std::string("adc") + std::to_string(iTS), 
                                        hfDigiTable->adcs_[iTS], 
                                        std::string("adc") + std::to_string(iTS));
        hfNanoTable->addColumn<int>(std::string("tdc") + std::to_string(iTS), 
                                        hfDigiTable->tdcs_[iTS], 
                                        std::string("tdc") + std::to_string(iTS));
        //hfNanoTable->addColumn<int>(std::string("tetdc") + std::to_string(iTS), 
        //                                hfDigiTable->tetdcs_[iTS], 
        //                                std::string("tetdc") + std::to_string(iTS));
        hfNanoTable->addColumn<int>(std::string("capid") + std::to_string(iTS), 
                                        hfDigiTable->capids_[iTS], 
                                        std::string("capid") + std::to_string(iTS));
        hfNanoTable->addColumn<float>(std::string("fc") + std::to_string(iTS), 
                                        hfDigiTable->fcs_[iTS], 
                                        std::string("fc") + std::to_string(iTS));
        hfNanoTable->addColumn<float>(std::string("pedestalfc") + std::to_string(iTS), 
                                        hfDigiTable->pedestalfcs_[iTS], 
                                        std::string("pedestalfc") + std::to_string(iTS));
        hfNanoTable->addColumn<float>(std::string("ok") + std::to_string(iTS), 
                                        hfDigiTable->oks_[iTS], 
                                        std::string("ok") + std::to_string(iTS));
    }
    iEvent.put(std::move(hfNanoTable));


    // HO
    auto hoNanoTable = std::make_unique<nanoaod::FlatTable>(nDigis_[HcalOuter], "HODigis", false, false);
    hoNanoTable->addColumn<int>("rawId", hoDigiTable->rawIds_, "rawId");
    hoNanoTable->addColumn<int>("ieta", hoDigiTable->ietas_, "ieta");
    hoNanoTable->addColumn<int>("iphi", hoDigiTable->iphis_, "iphi");
    hoNanoTable->addColumn<int>("depth", hoDigiTable->depths_, "depth");
    hoNanoTable->addColumn<int>("subdet", hoDigiTable->subdets_, "subdet");
    hoNanoTable->addColumn<int>("fiberIdleOffset", hoDigiTable->fiberIdleOffsets_, "fiberIdleOffset");
    hoNanoTable->addColumn<int>("soi", hoDigiTable->sois_, "soi");
    hoNanoTable->addColumn<bool>("valid", hoDigiTable->valids_, "valid");

    for (unsigned int iTS = 0; iTS < 10; ++iTS) {
        hoNanoTable->addColumn<int>(std::string("adc") + std::to_string(iTS), 
                                        hoDigiTable->adcs_[iTS], 
                                        std::string("adc") + std::to_string(iTS));
        hoNanoTable->addColumn<int>(std::string("capid") + std::to_string(iTS), 
                                        hoDigiTable->capids_[iTS], 
                                        std::string("capid") + std::to_string(iTS));
        hoNanoTable->addColumn<float>(std::string("fc") + std::to_string(iTS), 
                                        hoDigiTable->fcs_[iTS], 
                                        std::string("fc") + std::to_string(iTS));
        hoNanoTable->addColumn<float>(std::string("pedestalfc") + std::to_string(iTS), 
                                        hoDigiTable->pedestalfcs_[iTS], 
                                        std::string("pedestalfc") + std::to_string(iTS));
        hoNanoTable->addColumn<int>(std::string("fiber") + std::to_string(iTS), 
                                        hoDigiTable->fibers_[iTS], 
                                        std::string("fiber") + std::to_string(iTS));
        hoNanoTable->addColumn<int>(std::string("fiberChan") + std::to_string(iTS), 
                                        hoDigiTable->fiberChans_[iTS], 
                                        std::string("fiberChan") + std::to_string(iTS));
        hoNanoTable->addColumn<int>(std::string("dv") + std::to_string(iTS), 
                                        hoDigiTable->dvs_[iTS], 
                                        std::string("dv") + std::to_string(iTS));
        hoNanoTable->addColumn<int>(std::string("er") + std::to_string(iTS), 
                                        hoDigiTable->ers_[iTS], 
                                        std::string("er") + std::to_string(iTS));
    }
    iEvent.put(std::move(hoNanoTable));

}

#include "FWCore/Framework/interface/MakerMacros.h"
//define this as a plug-in
DEFINE_FWK_MODULE(HcalDigiTableProducer);

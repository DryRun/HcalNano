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
#include "DataFormats/HcalNano/interface/HcalChannelInfo.h"

namespace hcalnano {
    class HcalDigiTableProducer : public edm::stream::EDProducer<> {
    private:
        std::map<HcalSubdetector, std::vector<HcalDetId> > dids_;
        //std::map<HcalSubdetector, std::vector<HcalElectronicsId> > eids_;
        static const std::vector<HcalSubdetector> subdets_;
        HcalElectronicsMap const *emap_;

        edm::InputTag tagChannelInfo_;
        edm::EDGetTokenT<hcalnano::HcalChannelInfo> tokenChannelInfo_;

        edm::InputTag tagQIE11_;
        edm::EDGetTokenT<QIE11DigiCollection> tokenQIE11_;

        edm::InputTag tagQIE10_;
        edm::EDGetTokenT<QIE10DigiCollection> tokenQIE10_;

        edm::InputTag tagHO_;
        edm::EDGetTokenT<HODigiCollection> tokenHO_;


        edm::ESGetToken<HcalDbService, HcalDbRecord> tokenHcalDbService_;
        edm::ESHandle<HcalDbService> dbService_;


        hcalnano::HBDigiTable *hbDigiTable_;
        hcalnano::HEDigiTable *heDigiTable_;
        hcalnano::HFDigiTable *hfDigiTable_;
        hcalnano::HODigiTable *hoDigiTable_;

    public:
      explicit HcalDigiTableProducer(const edm::ParameterSet& iConfig) : 
        tokenChannelInfo_(consumes<edm::InRun>(iConfig.getUntrackedParameter<edm::InputTag>("tagChannelInfo", edm::InputTag("hcalChannelInfoTable")))),
        //tagChannelInfo_(iConfig.getUntrackedParameter<edm::InputTag>("tagChannelInfo", edm::InputTag(""))), 
        tagQIE11_(iConfig.getUntrackedParameter<edm::InputTag>("tagQIE11", edm::InputTag("hcalDigis"))), 
        tagQIE10_(iConfig.getUntrackedParameter<edm::InputTag>("tagQIE10", edm::InputTag("hcalDigis"))), 
        tagHO_(iConfig.getUntrackedParameter<edm::InputTag>("tagHO", edm::InputTag("hcalDigis"))), 
        tokenHcalDbService_(esConsumes<HcalDbService, HcalDbRecord, edm::Transition::BeginRun>())
        {

        //tokenChannelInfo_ = consumes<hcalnano::HcalChannelInfo, edm::InRun>(tagChannelInfo_);

        tokenQIE11_ = consumes<QIE11DigiCollection>(tagQIE11_);
        tokenHO_    = consumes<HODigiCollection>(tagHO_);
        tokenQIE10_ = consumes<QIE10DigiCollection>(tagQIE10_);

        produces<nanoaod::FlatTable>("HBDigiTable");
        produces<nanoaod::FlatTable>("HEDigiTable");
        produces<nanoaod::FlatTable>("HFDigiTable");
        produces<nanoaod::FlatTable>("HODigiTable");

        //tagQIE11_ = iConfig.getUntrackedParameter<edm::InputTag>("tagQIE11", edm::InputTag("hcalDigis"));
        //tagHO_ = iConfig.getUntrackedParameter<edm::InputTag>("tagHO", edm::InputTag("hcalDigis"));
        //tagQIE10_ = iConfig.getUntrackedParameter<edm::InputTag>("tagHF", edm::InputTag("hcalDigis"));



      }

        ~HcalDigiTableProducer() {
            delete hbDigiTable_;
            delete heDigiTable_;
            delete hfDigiTable_;
            delete hoDigiTable_;
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
}

const std::vector<HcalSubdetector> hcalnano::HcalDigiTableProducer::subdets_ = {HcalBarrel, HcalEndcap, HcalForward, HcalOuter};

void hcalnano::HcalDigiTableProducer::beginRun(edm::Run const& iRun, edm::EventSetup const& iSetup) {
    // List DetIds of interest from emap
    dbService_ = iSetup.getHandle(tokenHcalDbService_);
    emap_ = dbService_->getHcalMapping();

    //std::vector<HcalGenericDetId> dids = emap_->allPrecisionId();
    edm::Handle<hcalnano::HcalChannelInfo> channelInfo;
    iRun.getByToken(tokenChannelInfo_, channelInfo);

    dids_ = channelInfo->dids;
    //eids_ = channelInfo->eids;

    // Create persistent digi storage
    hbDigiTable_ = new hcalnano::HBDigiTable(dids_[HcalBarrel], 8);
    heDigiTable_ = new hcalnano::HEDigiTable(dids_[HcalEndcap], 8);
    hfDigiTable_ = new hcalnano::HFDigiTable(dids_[HcalForward], 3);
    hoDigiTable_ = new hcalnano::HODigiTable(dids_[HcalOuter], 10);
}


void hcalnano::HcalDigiTableProducer::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {
    // * Load digis */
    edm::Handle<QIE11DigiCollection> qie11Digis;
    iEvent.getByToken(tokenQIE11_, qie11Digis);

    edm::Handle<QIE10DigiCollection> qie10Digis;
    iEvent.getByToken(tokenQIE10_, qie10Digis);

    edm::Handle<HODigiCollection> hoDigis;
    iEvent.getByToken(tokenHO_, hoDigis);

    // * Process digis */
    // HB
    hbDigiTable_->reset();
    for (QIE11DigiCollection::const_iterator itDigi = qie11Digis->begin(); itDigi != qie11Digis->end(); ++itDigi) {
        const QIE11DataFrame digi = static_cast<const QIE11DataFrame>(*itDigi);
        HcalDetId const& did = digi.detid();
        if (did.subdet() != HcalBarrel) continue;

        hbDigiTable_->add(&digi, dbService_);
    } // End loop over qie11 HB digis


    // HE
    heDigiTable_->reset();
    for (QIE11DigiCollection::const_iterator itDigi = qie11Digis->begin(); itDigi != qie11Digis->end(); ++itDigi) {
        const QIE11DataFrame digi = static_cast<const QIE11DataFrame>(*itDigi);
        HcalDetId const& did = digi.detid();
        if (did.subdet() != HcalEndcap) continue;

        heDigiTable_->add(&digi, dbService_);
    } // End loop over qie11 HE digis


    // HF
    hfDigiTable_->reset();
    for (QIE10DigiCollection::const_iterator itDigi = qie10Digis->begin(); itDigi != qie10Digis->end(); ++itDigi) {
        const QIE10DataFrame digi = static_cast<const QIE10DataFrame>(*itDigi);
        HcalDetId const& did = digi.detid();
        if (did.subdet() != HcalForward) continue;

        hfDigiTable_->add(&digi, dbService_);
    } // End loop over qie10 HF digis


    // HO
    hoDigiTable_->reset();
    for (HODigiCollection::const_iterator itDigi = hoDigis->begin(); itDigi != hoDigis->end(); ++itDigi) {
        const HODataFrame digi = static_cast<const HODataFrame>(*itDigi);
        HcalDetId const& did = digi.id();
        if (did.subdet() != HcalOuter) continue;

        hoDigiTable_->add(&digi, dbService_);
    } // End loop over HO digis


    // * Save to NanoAOD tables */

    // HB
    auto hbNanoTable = std::make_unique<nanoaod::FlatTable>(dids_[HcalBarrel].size(), "HBDigis", false, false);
    hbNanoTable->addColumn<int>("rawId", hbDigiTable_->rawIds_, "rawId");
    hbNanoTable->addColumn<int>("ieta", hbDigiTable_->ietas_, "ieta");
    hbNanoTable->addColumn<int>("iphi", hbDigiTable_->iphis_, "iphi");
    hbNanoTable->addColumn<int>("depth", hbDigiTable_->depths_, "depth");
    hbNanoTable->addColumn<int>("subdet", hbDigiTable_->subdets_, "subdet");
    hbNanoTable->addColumn<bool>("linkError", hbDigiTable_->linkErrors_, "linkError");
    hbNanoTable->addColumn<bool>("capidError", hbDigiTable_->capidErrors_, "capidError");
    hbNanoTable->addColumn<int>("flags", hbDigiTable_->flags_, "flags");
    hbNanoTable->addColumn<int>("soi", hbDigiTable_->sois_, "soi");
    hbNanoTable->addColumn<bool>("valid", hbDigiTable_->valids_, "valid");

    for (unsigned int iTS = 0; iTS < 8; ++iTS) {
        hbNanoTable->addColumn<int>(std::string("adc") + std::to_string(iTS), 
                                        hbDigiTable_->adcs_[iTS], 
                                        std::string("adc") + std::to_string(iTS));
        hbNanoTable->addColumn<int>(std::string("tdc") + std::to_string(iTS), 
                                        hbDigiTable_->tdcs_[iTS], 
                                        std::string("tdc") + std::to_string(iTS));
        hbNanoTable->addColumn<int>(std::string("capid") + std::to_string(iTS), 
                                        hbDigiTable_->capids_[iTS], 
                                        std::string("capid") + std::to_string(iTS));
        hbNanoTable->addColumn<float>(std::string("fc") + std::to_string(iTS), 
                                        hbDigiTable_->fcs_[iTS], 
                                        std::string("fc") + std::to_string(iTS));
        hbNanoTable->addColumn<float>(std::string("pedestalfc") + std::to_string(iTS), 
                                        hbDigiTable_->pedestalfcs_[iTS], 
                                        std::string("pedestalfc") + std::to_string(iTS));
    }
    iEvent.put(std::move(hbNanoTable), "HBDigiTable");

    // HE
    auto heNanoTable = std::make_unique<nanoaod::FlatTable>(dids_[HcalEndcap].size(), "HEDigis", false, false);
    heNanoTable->addColumn<int>("rawId", heDigiTable_->rawIds_, "rawId");
    heNanoTable->addColumn<int>("ieta", heDigiTable_->ietas_, "ieta");
    heNanoTable->addColumn<int>("iphi", heDigiTable_->iphis_, "iphi");
    heNanoTable->addColumn<int>("depth", heDigiTable_->depths_, "depth");
    heNanoTable->addColumn<int>("subdet", heDigiTable_->subdets_, "subdet");
    heNanoTable->addColumn<bool>("linkError", heDigiTable_->linkErrors_, "linkError");
    heNanoTable->addColumn<bool>("capidError", heDigiTable_->capidErrors_, "capidError");
    heNanoTable->addColumn<int>("flags", heDigiTable_->flags_, "flags");
    heNanoTable->addColumn<int>("soi", heDigiTable_->sois_, "soi");
    heNanoTable->addColumn<bool>("valid", heDigiTable_->valids_, "valid");

    for (unsigned int iTS = 0; iTS < 8; ++iTS) {
        heNanoTable->addColumn<int>(std::string("adc") + std::to_string(iTS), 
                                        heDigiTable_->adcs_[iTS], 
                                        std::string("adc") + std::to_string(iTS));
        heNanoTable->addColumn<int>(std::string("tdc") + std::to_string(iTS), 
                                        heDigiTable_->tdcs_[iTS], 
                                        std::string("tdc") + std::to_string(iTS));
        heNanoTable->addColumn<int>(std::string("capid") + std::to_string(iTS), 
                                        heDigiTable_->capids_[iTS], 
                                        std::string("capid") + std::to_string(iTS));
        heNanoTable->addColumn<float>(std::string("fc") + std::to_string(iTS), 
                                        heDigiTable_->fcs_[iTS], 
                                        std::string("fc") + std::to_string(iTS));
        heNanoTable->addColumn<float>(std::string("pedestalfc") + std::to_string(iTS), 
                                        heDigiTable_->pedestalfcs_[iTS], 
                                        std::string("pedestalfc") + std::to_string(iTS));
    }
    iEvent.put(std::move(heNanoTable), "HEDigiTable");

    // HF
    auto hfNanoTable = std::make_unique<nanoaod::FlatTable>(dids_[HcalForward].size(), "HFDigis", false, false);
    hfNanoTable->addColumn<int>("rawId", hfDigiTable_->rawIds_, "rawId");
    hfNanoTable->addColumn<int>("ieta", hfDigiTable_->ietas_, "ieta");
    hfNanoTable->addColumn<int>("iphi", hfDigiTable_->iphis_, "iphi");
    hfNanoTable->addColumn<int>("depth", hfDigiTable_->depths_, "depth");
    hfNanoTable->addColumn<int>("subdet", hfDigiTable_->subdets_, "subdet");
    hfNanoTable->addColumn<bool>("linkError", hfDigiTable_->linkErrors_, "linkError");
    hfNanoTable->addColumn<int>("flags", hfDigiTable_->flags_, "flags");
    hfNanoTable->addColumn<int>("soi", hfDigiTable_->sois_, "soi");
    hfNanoTable->addColumn<bool>("valid", hfDigiTable_->valids_, "valid");

    for (unsigned int iTS = 0; iTS < 3; ++iTS) {
        hfNanoTable->addColumn<int>(std::string("adc") + std::to_string(iTS), 
                                        hfDigiTable_->adcs_[iTS], 
                                        std::string("adc") + std::to_string(iTS));
        hfNanoTable->addColumn<int>(std::string("tdc") + std::to_string(iTS), 
                                        hfDigiTable_->tdcs_[iTS], 
                                        std::string("tdc") + std::to_string(iTS));
        //hfNanoTable->addColumn<int>(std::string("tetdc") + std::to_string(iTS), 
        //                                hfDigiTable_->tetdcs_[iTS], 
        //                                std::string("tetdc") + std::to_string(iTS));
        hfNanoTable->addColumn<int>(std::string("capid") + std::to_string(iTS), 
                                        hfDigiTable_->capids_[iTS], 
                                        std::string("capid") + std::to_string(iTS));
        hfNanoTable->addColumn<float>(std::string("fc") + std::to_string(iTS), 
                                        hfDigiTable_->fcs_[iTS], 
                                        std::string("fc") + std::to_string(iTS));
        hfNanoTable->addColumn<float>(std::string("pedestalfc") + std::to_string(iTS), 
                                        hfDigiTable_->pedestalfcs_[iTS], 
                                        std::string("pedestalfc") + std::to_string(iTS));
        hfNanoTable->addColumn<float>(std::string("ok") + std::to_string(iTS), 
                                        hfDigiTable_->oks_[iTS], 
                                        std::string("ok") + std::to_string(iTS));
    }
    iEvent.put(std::move(hfNanoTable), "HFDigiTable");


    // HO
    auto hoNanoTable = std::make_unique<nanoaod::FlatTable>(dids_[HcalOuter].size(), "HODigis", false, false);
    hoNanoTable->addColumn<int>("rawId", hoDigiTable_->rawIds_, "rawId");
    hoNanoTable->addColumn<int>("ieta", hoDigiTable_->ietas_, "ieta");
    hoNanoTable->addColumn<int>("iphi", hoDigiTable_->iphis_, "iphi");
    hoNanoTable->addColumn<int>("depth", hoDigiTable_->depths_, "depth");
    hoNanoTable->addColumn<int>("subdet", hoDigiTable_->subdets_, "subdet");
    hoNanoTable->addColumn<int>("fiberIdleOffset", hoDigiTable_->fiberIdleOffsets_, "fiberIdleOffset");
    hoNanoTable->addColumn<int>("soi", hoDigiTable_->sois_, "soi");
    hoNanoTable->addColumn<bool>("valid", hoDigiTable_->valids_, "valid");

    for (unsigned int iTS = 0; iTS < 10; ++iTS) {
        hoNanoTable->addColumn<int>(std::string("adc") + std::to_string(iTS), 
                                        hoDigiTable_->adcs_[iTS], 
                                        std::string("adc") + std::to_string(iTS));
        hoNanoTable->addColumn<int>(std::string("capid") + std::to_string(iTS), 
                                        hoDigiTable_->capids_[iTS], 
                                        std::string("capid") + std::to_string(iTS));
        hoNanoTable->addColumn<float>(std::string("fc") + std::to_string(iTS), 
                                        hoDigiTable_->fcs_[iTS], 
                                        std::string("fc") + std::to_string(iTS));
        hoNanoTable->addColumn<float>(std::string("pedestalfc") + std::to_string(iTS), 
                                        hoDigiTable_->pedestalfcs_[iTS], 
                                        std::string("pedestalfc") + std::to_string(iTS));
        hoNanoTable->addColumn<int>(std::string("fiber") + std::to_string(iTS), 
                                        hoDigiTable_->fibers_[iTS], 
                                        std::string("fiber") + std::to_string(iTS));
        hoNanoTable->addColumn<int>(std::string("fiberChan") + std::to_string(iTS), 
                                        hoDigiTable_->fiberChans_[iTS], 
                                        std::string("fiberChan") + std::to_string(iTS));
        hoNanoTable->addColumn<int>(std::string("dv") + std::to_string(iTS), 
                                        hoDigiTable_->dvs_[iTS], 
                                        std::string("dv") + std::to_string(iTS));
        hoNanoTable->addColumn<int>(std::string("er") + std::to_string(iTS), 
                                        hoDigiTable_->ers_[iTS], 
                                        std::string("er") + std::to_string(iTS));
    }
    iEvent.put(std::move(hoNanoTable), "HODigiTable");

}

#include "FWCore/Framework/interface/MakerMacros.h"
//define this as a plug-in
DEFINE_FWK_MODULE(hcalnano::HcalDigiTableProducer);

// system include files
#include <memory>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"

#include "FWCore/Framework/interface/Event.h"

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


//#include "HCALPFG/HcalNano/interface/QIE10Digi.h"
#include "HCALPFG/HCALNano/interface/QIE11DigiTable.h"

class HcalDigiTableProducer : public edm::stream::EDProducer<> {
private:
    std::map<HcalSubdetector, std::vector<HcalDetId> > dids_;
    static const std::vector<HcalSubdetector> subdets_;
    edm::ESHandle<HcalDbService> dbService_;
    HcalElectronicsMap const *emap_;
    std::map<HcalSubdetector, unsigned int> nDigis_;

    edm::InputTag tagQIE11_;
    edm::EDGetTokenT<QIE11DigiCollection> tokenQIE11_;

    HBDigiTable *hbDigiTable;

public:
  explicit HcalDigiTableProducer(const edm::ParameterSet& iConfig) {
    produces<nanoaod::FlatTable>();

    tagQIE11_ = iConfig.getUntrackedParameter<edm::InputTag>("tagQIE11", edm::InputTag("hcalDigis"));
    //tagHO_ = iConfig.getUntrackedParameter<edm::InputTag>("tagHO", edm::InputTag("hcalDigis"));
    //tagQIE10_ = iConfig.getUntrackedParameter<edm::InputTag>("tagHF", edm::InputTag("hcalDigis"));

    tokenQIE11_ = consumes<QIE11DigiCollection>(tagQIE11_);
    //tokHO_ = consumes<HODigiCollection>(_tagHO);
    //tokQIE10_ = consumes<QIE10DigiCollection>(_tagQIE10);

  }

    ~HcalDigiTableProducer() {
        delete hbDigiTable;
    };

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
    edm::ParameterSetDescription desc;
    //desc.add<edm::InputTag>("src")->setComment("input digi collection");
    // desc.add<std::string>("name")->setComment("");
    descriptions.add("HcalDigiTable", desc);
  }

private:
    void beginRun(edm::Run const&, edm::EventSetup const&);
    void produce(edm::Event&, edm::EventSetup const&) override;

};

const std::vector<HcalSubdetector> HcalDigiTableProducer::subdets_ = {HcalBarrel, HcalEndcap, HcalForward, HcalOuter};

void HcalDigiTableProducer::beginRun(edm::Run const&, edm::EventSetup const&) {
    // List DetIds of interest from emap
    edm::ESHandle<HcalDbService> dbService_;
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
    for (auto& it_subdet : subdets_) {
        std::sort(dids_[it_subdet].begin(), dids_[it_subdet].end());
    }

    // Create persistent digi storage
    hbDigiTable = new HBDigiTable(dids_[HcalBarrel], 8);
}


void HcalDigiTableProducer::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {
    hbDigiTable->reset();
    edm::Handle<QIE11DigiCollection> qie11Digis;
    iEvent.getByToken(tokenQIE11_, qie11Digis);

    for (QIE11DigiCollection::const_iterator it_rawdigi = qie11Digis->begin(); it_rawdigi != qie11Digis->end(); ++it_rawdigi) {
        const QIE11DataFrame inputDigi = static_cast<const QIE11DataFrame>(*it_rawdigi);
        HcalDetId const& did = inputDigi.detid();
        if (did.subdet() != HcalBarrel) continue;

        hbDigiTable->add(inputDigi, dbService_);
    } // End loop over qie11 digis

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
}

#include "FWCore/Framework/interface/MakerMacros.h"
//define this as a plug-in
DEFINE_FWK_MODULE(HcalDigiTableProducer);

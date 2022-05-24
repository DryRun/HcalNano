// system include files
#include <memory>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/global/EDProducer.h"
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

#include "DataFormats/HcalNano/interface/HcalChannelInfo.h"

typedef std::vector<HcalDetId> HcalDIDList;
typedef std::vector<HcalElectronicsId> HcalEIDList;

namespace hcalnano {

	class HcalChannelInfoTableProducer : public edm::global::EDProducer<edm::BeginRunProducer> {
	private:
		//std::unique_ptr<HcalChannelInfo> channelInfo;
		//static const std::vector<HcalSubdetector> subdets;
	    edm::ESGetToken<HcalDbService, HcalDbRecord> tokenHcalDbService_;
	    edm::EDPutTokenT<hcalnano::HcalChannelInfo> channelInfoToken_;
	public:
	  explicit HcalChannelInfoTableProducer(const edm::ParameterSet& iConfig) : 
	    tokenHcalDbService_(esConsumes<HcalDbService, HcalDbRecord, edm::Transition::BeginRun>())
	    {
	    //produces<HcalChannelInfo, edm::Transition::BeginRun>("");
		channelInfoToken_ = produces<hcalnano::HcalChannelInfo, edm::Transition::BeginRun>("");
	    produces<nanoaod::FlatTable, edm::Transition::BeginRun>("HBChannelInfo");
	    produces<nanoaod::FlatTable, edm::Transition::BeginRun>("HEChannelInfo");
	    produces<nanoaod::FlatTable, edm::Transition::BeginRun>("HFChannelInfo");
	    produces<nanoaod::FlatTable, edm::Transition::BeginRun>("HOChannelInfo");
		};

		void produce(edm::StreamID, edm::Event&, edm::EventSetup const&) const;
		void globalBeginRunProduce(edm::Run& iRun, edm::EventSetup const& iSetup) const;

	};
};

void hcalnano::HcalChannelInfoTableProducer::produce(edm::StreamID, edm::Event&, edm::EventSetup const&) const {}

void hcalnano::HcalChannelInfoTableProducer::globalBeginRunProduce(edm::Run& iRun, edm::EventSetup const& iSetup) const {
	// Setup products
	const std::vector<HcalSubdetector> subdets = {HcalBarrel, HcalEndcap, HcalForward, HcalOuter};
	std::unique_ptr<HcalChannelInfo> channelInfo = std::make_unique<HcalChannelInfo>();

	// Load channels from emap
    edm::ESHandle<HcalDbService> dbService = iSetup.getHandle(tokenHcalDbService_);
    HcalElectronicsMap const *emap = dbService->getHcalMapping();

    std::vector<HcalGenericDetId> alldids = emap->allPrecisionId();
    for (auto it_did = alldids.begin(); it_did != alldids.end(); ++it_did) {
        if (!it_did->isHcalDetId()) {
            continue;
        }
        HcalDetId did = HcalDetId(it_did->rawId());
        if (!(did.subdet() == HcalBarrel || did.subdet() == HcalEndcap || did.subdet() == HcalForward || did.subdet() == HcalOuter)) {
            continue;
        }

        // TODO: Add filtering, for example on FED whitelist
        
        channelInfo->dids[did.subdet()].push_back(did);
    }

    // Sort HcalDetIds
    for (auto& it_subdet : subdets) {
        std::sort(channelInfo->dids[it_subdet].begin(), channelInfo->dids[it_subdet].end());
    }

    // EIDs
    for (auto& it_subdet : subdets) {
        for (auto it_did : channelInfo->dids[it_subdet]) {
            channelInfo->eids[it_subdet].push_back(HcalElectronicsId(it_did.rawId()));
        }
    }


    // Make NanoAOD tables
    std::map<HcalSubdetector, std::string> subdetNames;
	subdetNames[HcalBarrel]  = "HB";
	subdetNames[HcalEndcap]  = "HE";
	subdetNames[HcalForward] = "HF";
	subdetNames[HcalOuter]   = "HO";
	
	for (auto& it_subdet : subdets) {
		auto channelTable = std::make_unique<nanoaod::FlatTable>(channelInfo->dids[it_subdet].size(), subdetNames[it_subdet], false, false);

		std::vector<int> vdids;
		for (auto& it_did : channelInfo->dids[it_subdet]) {
			vdids.push_back(it_did.rawId());
		}
		channelTable->addColumn<int>("did", 
								vdids, 
								"HcalDetId");

		std::vector<int> veids;
		for (auto& it_eid : channelInfo->eids[it_subdet]) {
			veids.push_back(it_eid.rawId());
		}
		channelTable->addColumn<int>("eid", 
								veids,
								"HcalElectronicsId");
		iRun.put(std::move(channelTable), subdetNames[it_subdet] + "ChannelInfo");
	}
    // Add to run data
    iRun.put(std::move(channelInfo), "");

}

#include "FWCore/Framework/interface/MakerMacros.h"
//define this as a plug-in
DEFINE_FWK_MODULE(hcalnano::HcalChannelInfoTableProducer);

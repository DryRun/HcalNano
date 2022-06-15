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
    class HcalEventTableProducer : public edm::stream::EDProducer<> {
    private:

    public:
      explicit HcalEventTableProducer(const edm::ParameterSet& iConfig)
        {
        produces<nanoaod::FlatTable>();
      }

        ~HcalEventTableProducer() {
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
        void produce(edm::Event&, edm::EventSetup const&) override;

    };
}

void hcalnano::HcalEventTableProducer::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {
    auto hcalEventTable = std::make_unique<nanoaod::FlatTable>(1, "", true);
    hcalEventTable->addColumnValue<int>("bunchCrossing", iEvent.bunchCrossing(), "Bunch crossing ID");
    hcalEventTable->addColumnValue<int>("orbitNumber", iEvent.orbitNumber(), "Orbit number");
    iEvent.put(std::move(hcalEventTable));
}

#include "FWCore/Framework/interface/MakerMacros.h"
//define this as a plug-in
DEFINE_FWK_MODULE(hcalnano::HcalEventTableProducer);

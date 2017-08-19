/**
 * @file DetIDAnalyzer.cxx
 * @brief Defines class for checking that hit IDs can be matched with their detector elements
 * @author Jeremy McCormick, SLAC
 */

#include "Framework/EventProcessor.h"
#include "Framework/ParameterSet.h"

#include "Event/Event.h"
#include "Event/EventConstants.h"
#include "Event/CalorimeterHit.h"
#include "Event/SimTrackerHit.h"

#include "DetDescr/EcalDetectorID.h"
#include "DetDescr/HcalID.h"
#include "DetDescr/TrackerID.h"

#include <iostream>

#include "TClonesArray.h"

namespace ldmx {

    /**
     * @class DetIDAnalyzer
     * @brief Checks that that IDs can be matched with their detector elements
     */
    class DetIDAnalyzer : public ldmx::Analyzer {

        public:

            DetIDAnalyzer(const std::string& name, ldmx::Process& process) : ldmx::Analyzer(name, process) {}

            virtual void configure(const ldmx::ParameterSet& ps) {
            }

            virtual void analyze(const ldmx::Event& event) {
                std::cout << "DetIDAnalyzer: analyzing event <" << event.getEventHeader()->getEventNumber() << ">" << std::endl;

                // Check hit IDs for ecal hits.
                const TClonesArray* ecalSimHits = event.getCollection(EventConstants::ECAL_SIM_HITS);
                for (size_t i=0; i<ecalSimHits->GetEntriesFast(); i++) {
                    const ldmx::CalorimeterHit* chit=(const ldmx::CalorimeterHit*)(ecalSimHits->At(i));

                    ecalID_->setRawValue(chit->getID());
                    auto hitID = ecalID_->unpack();
                    std::cout << "Ecal hit: " << hitID << std::endl;

                    // Strip cell ID so the lookup will work.
                    ecalID_->clear();
                    ecalID_->setFieldValue(0, hitID[0]);
                    ecalID_->setFieldValue(1, hitID[1]);
                    ecalID_->setFieldValue(2, hitID[2]);
                    int moduleID = ecalID_->pack();
                    auto moduleFields = ecalID_->unpack();
                    std::cout << "Module ID: " << moduleFields << std::endl;

                    DetectorElement* de = detSvc_->getDetectorElement(moduleID);
                    if (de) {
                        std::cout << "Found matching DE for cal hit: " << de->getName() << std::endl;
                    } else {
                        EXCEPTION_RAISE("DetIDAnalyzer", "Failed to find DetectorElement for CalorimeterHit!");
                    }
                    std::cout << std::endl;
                }

                // Check hit IDs for hcal hits.
                const TClonesArray* hcalSimHits = event.getCollection(EventConstants::HCAL_SIM_HITS);
                for (size_t i=0; i<hcalSimHits->GetEntriesFast(); i++) {
                    const ldmx::CalorimeterHit* chit=(const ldmx::CalorimeterHit*)(hcalSimHits->At(i));

                    hcalID_->setRawValue(chit->getID());
                    auto hitID = hcalID_->unpack();
                    std::cout << "Hcal hit: " << hitID << std::endl;

                    // Remove strip ID so the lookup will work.
                    hcalID_->clear();
                    hcalID_->setFieldValue(0, hitID[0]);
                    hcalID_->setFieldValue(1, hitID[1]);
                    ecalID_->setFieldValue(2, hitID[2]);
                    int id = hcalID_->pack();
                    auto values = hcalID_->unpack();
                    std::cout << "Module ID: " << values << std::endl;

                    DetectorElement* de = detSvc_->getDetectorElement(id);
                    if (de) {
                        std::cout << "Found DE for hcal hit: " << de->getName() << std::endl;
                    } else {
                        EXCEPTION_RAISE("DetIDAnalyzer", "Failed to find DetectorElement for CalorimeterHit!");
                    }
                    std::cout << std::endl;
                }

                // Check hit IDs for Recoil det tracker hits.
                const TClonesArray* recoilSimHits = event.getCollection(EventConstants::RECOIL_SIM_HITS);
                for (size_t i=0; i<recoilSimHits->GetEntriesFast(); i++) {
                    const ldmx::SimTrackerHit* thit = (const ldmx::SimTrackerHit*)(recoilSimHits->At(i));
                 
                    recoilID_->setRawValue(thit->getID());
                    auto hitID = recoilID_->unpack();
                    std::cout << "Recoil hit: " << hitID << std::endl; 

                    DetectorElement* de = detSvc_->getDetectorElement(thit->getID());
                    if (de) {
                        std::cout << "Found matching DE for tkr hit: " << de->getName() << std::endl;
                    } else {
                        EXCEPTION_RAISE("DetIDAnalyzer", "Failed to find DetectorElement for SimTrackerHit!");
                    }
                    std::cout << std::endl;
                } 

                // Check hit IDs for tagger hits.
                const TClonesArray* taggerSimHits = event.getCollection(EventConstants::TAGGER_SIM_HITS);
                for (size_t i=0; i<taggerSimHits->GetEntriesFast(); i++) {
                    const ldmx::SimTrackerHit* thit = (const ldmx::SimTrackerHit*)(taggerSimHits->At(i));
                 
                    taggerID_->setRawValue(thit->getID());
                    auto hitID = taggerID_->unpack();
                    std::cout << "Tagger hit: " << hitID << std::endl; 

                    DetectorElement* de = detSvc_->getDetectorElement(thit->getID());
                    if (de) {
                        std::cout << "Found matching DE for tkr hit: " << de->getName() << std::endl;
                    } else {
                        EXCEPTION_RAISE("DetIDAnalyzer", "Failed to find DetectorElement for SimTrackerHit!");
                    }
                    std::cout << std::endl;
                } 
                
            }

            void onNewDetector(DetectorDataService* detSvc) {

                std::cout << "[DetIDAnalyzer] got new detector - " << detSvc->getDetectorName() << std::endl;

                if (detSvc) {
                    detSvc_ = detSvc;
                    top_ = detSvc->getTopDetectorElement();

                    ecalID_ = top_->findChild("Ecal")->getDetectorID();
                    if (!ecalID_) {
                        EXCEPTION_RAISE("DetIDAnalyzer", "EcalID not found.");
                    }

                    hcalID_ = top_->findChild("Hcal")->getDetectorID();
                    if (!hcalID_) {
                        EXCEPTION_RAISE("DetIDAnalyzer", "HcalID not found.");
                    }

                    recoilID_ = top_->findChild("RecoilTracker")->getDetectorID();
                    if (!recoilID_) {
                        EXCEPTION_RAISE("DetIDAnalyzer", "RecoilTracker ID not found.");
                    }

                    taggerID_ = top_->findChild("Tagger")->getDetectorID();
                    if (!taggerID_) {
                        EXCEPTION_RAISE("DetIDAnalyzer", "Tagger ID not found.");
                    }

                } else {
                    EXCEPTION_RAISE("DetIDAnalyzer", "The detector service is not setup!");
                }
            }

        private:

            DetectorDataService* detSvc_{nullptr};
            DetectorElement* top_{nullptr};

            DetectorID* ecalID_{nullptr};
            DetectorID* hcalID_{nullptr};
            DetectorID* recoilID_{nullptr};
            DetectorID* taggerID_{nullptr};
    };
}

DECLARE_ANALYZER_NS(ldmx, DetIDAnalyzer);

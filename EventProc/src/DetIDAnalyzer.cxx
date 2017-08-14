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

                // Check hit IDs for cal hits.
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

                // Check hit IDs for Recoil det tracker hits.
                const TClonesArray* recoilSimHits = event.getCollection(EventConstants::RECOIL_SIM_HITS);
                for (size_t i=0; i<recoilSimHits->GetEntriesFast(); i++) {
                    const ldmx::SimTrackerHit* thit = (const ldmx::SimTrackerHit*)(recoilSimHits->At(i));
                 
                    detID_->setRawValue(thit->getID());
                    auto hitID = detID_->unpack();   
                    std::cout << "Recoil hit: " << hitID << std::endl; 

                    DetectorElement* de = detSvc_->getDetectorElement(thit->getID());
                    if (de) {
                        std::cout << "Found matching DE for tkr hit: " << de->getName() << std::endl;
                    } else {
                        EXCEPTION_RAISE("DetIDAnalyzer", "Failed to find DetectorElement for SimTrackerHit!");
                    }
                    std::cout << std::endl;
                } 

                // Check hit IDs for Recoil det tracker hits.
                const TClonesArray* taggerSimHits = event.getCollection(EventConstants::TAGGER_SIM_HITS);
                for (size_t i=0; i<taggerSimHits->GetEntriesFast(); i++) {
                    const ldmx::SimTrackerHit* thit = (const ldmx::SimTrackerHit*)(taggerSimHits->At(i));
                 
                    detID_->setRawValue(thit->getID());
                    auto hitID = detID_->unpack();   
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
                if (detSvc) {
                    detSvc_ = detSvc;
                    top_ = detSvc->getTopDetectorElement();
                } else {
                    EXCEPTION_RAISE("DetIDAnalyzer", "The detector service is null!");
                }
            }

        private:
            DetectorDataService* detSvc_{nullptr};
            DetectorElement* top_{nullptr};
            EcalDetectorID* ecalID_{new EcalDetectorID};
            DetectorID* detID_{new DefaultDetectorID};
    };
}

DECLARE_ANALYZER_NS(ldmx, DetIDAnalyzer);

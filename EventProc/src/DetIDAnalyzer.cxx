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
                const TClonesArray* tca=event.getCollection(EventConstants::ECAL_SIM_HITS);
                for (size_t i=0; i<tca->GetEntriesFast(); i++) {
                    const ldmx::CalorimeterHit* chit=(const ldmx::CalorimeterHit*)(tca->At(i));

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
                        std::cout << "Found matching DE: " << de->getName() << std::endl;
                    } else {
                        EXCEPTION_RAISE("DetIDAnalyzer", "Failed to find DetectorElement for CalorimeterHit!");
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
    };
}

DECLARE_ANALYZER_NS(ldmx, DetIDAnalyzer);

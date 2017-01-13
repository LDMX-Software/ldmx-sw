/**
 * @file ExampleCalHitProcessor.h
 * @brief Class containing an example SimCalorimeterHit processor
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef EVENTPROC_EXAMPLECALHITPROCESSOR_H_
#define EVENTPROC_EXAMPLECALHITPROCESSOR_H_

#include "Event/CalorimeterHit.h"
#include "Event/SimCalorimeterHit.h"

namespace eventproc {

/**
 * @class ExampleCalHitProcessor
 * @brief Example EventProcessor that copies a SimCalorimeterHit collection into a new output collection
 */
class ExampleCalHitProcessor : public EventProcessor {

    public:

        void initialize() {
        }

        void execute() {
            ecalHits_->Clear("");
            auto ecalSimHits = event_->getCollection(event::EventConstants::ECAL_SIM_HITS, "sim");
            for (int i = 0; i < ecalSimHits->GetEntriesFast(); i++) {
                auto simHit = (event::SimCalorimeterHit*) ecalSimHits->At(i);
                auto calHit = (event::CalorimeterHit*) ecalHits_->ConstructedAt(ecalHits_->GetEntries());
                calHit->setID(simHit->getID());
                calHit->setEdep(simHit->getEdep());
                calHit->setTime(simHit->getTime());
            }
            event_->add("EcalCalHits", ecalHits_);
        }

        void finish() {
        }

    public:

        TClonesArray* ecalHits_{new TClonesArray("event::CalorimeterHit", 50)};
};

}

#endif /* EVENTPROC_EXAMPLECALHITPROCESSOR_H_ */

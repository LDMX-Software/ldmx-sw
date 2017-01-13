#ifndef EVENTPROC_EXAMPLECALHITPROCESSOR_H_
#define EVENTPROC_EXAMPLECALHITPROCESSOR_H_

#include "Event/SimCalorimeterHit.h"

namespace eventproc {

class ExampleCalHitProcessor : public EventProcessor {

    public:

        void initialize() {
        }

        void execute() {
            auto ecalSimHits = event_->getCollection(event::EventConstants::ECAL_SIM_HITS);
            for (int i = 0; i < ecalSimHits->GetEntriesFast(); i++) {
                auto simHit = (event::SimCalorimeterHit*) ecalSimHits->At(i);
                auto calHit = (event::SimCalorimeterHit*) ecalHits_->ConstructedAt(ecalHits_->GetEntries());
                calHit->setID(simHit->getID());
                calHit->setEdep(simHit->getEdep());
                calHit->setTime(simHit->getTime());
                auto pos = simHit->getPosition();
                calHit->setPosition(pos[0], pos[1], pos[2]);
            }
            event_->add("EcalCalHits", ecalHits_);
        }

        void finish() {
            //delete ecalHits_;
        }

    public:

        TClonesArray* ecalHits_{new TClonesArray("event::SimCalorimeterHit", 50)};
};

}

#endif /* EVENTPROC_EXAMPLECALHITPROCESSOR_H_ */

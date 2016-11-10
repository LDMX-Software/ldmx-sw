#include "Event/SimEvent.h"

ClassImp(event::SimEvent)

using event::EventConstants;

namespace event {

SimEvent::SimEvent() :
        simParticles_     (new TClonesArray(EventConstants::SIM_PARTICLE,        EventConstants::DEFAULT_COLLECTION_SIZE)),
        taggerSimHits_    (new TClonesArray(EventConstants::SIM_TRACKER_HIT,     EventConstants::DEFAULT_COLLECTION_SIZE)),
        recoilSimHits_    (new TClonesArray(EventConstants::SIM_TRACKER_HIT,     EventConstants::DEFAULT_COLLECTION_SIZE)),
        ecalSimHits_      (new TClonesArray(EventConstants::SIM_CALORIMETER_HIT, EventConstants::DEFAULT_COLLECTION_SIZE)),
        hcalSimHits_      (new TClonesArray(EventConstants::SIM_CALORIMETER_HIT, EventConstants::DEFAULT_COLLECTION_SIZE)),
        targetSimHits_    (new TClonesArray(EventConstants::SIM_CALORIMETER_HIT, EventConstants::DEFAULT_COLLECTION_SIZE)),
        triggerPadSimHits_(new TClonesArray(EventConstants::SIM_CALORIMETER_HIT, EventConstants::DEFAULT_COLLECTION_SIZE)) {

    collMap_[EventConstants::RECOIL_SIM_HITS] = recoilSimHits_;
    collMap_[EventConstants::TAGGER_SIM_HITS] = taggerSimHits_;
    collMap_[EventConstants::SIM_PARTICLES] = simParticles_;
    collMap_[EventConstants::ECAL_SIM_HITS] = ecalSimHits_;
    collMap_[EventConstants::HCAL_SIM_HITS] = hcalSimHits_;
    collMap_[EventConstants::TRIGGER_PAD_SIM_HITS] = triggerPadSimHits_;
    collMap_[EventConstants::TARGET_SIM_HITS] = targetSimHits_;
}

}

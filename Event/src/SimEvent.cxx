#include "Event/SimEvent.h"

ClassImp(event::SimEvent)

namespace event {

SimEvent::SimEvent() :
        simParticles_(new TClonesArray(event::SIM_PARTICLE, event::DEFAULT_COLLECTION_SIZE)),
        taggerSimHits_(new TClonesArray(event::SIM_TRACKER_HIT, event::DEFAULT_COLLECTION_SIZE)),
        recoilSimHits_(new TClonesArray(event::SIM_TRACKER_HIT, event::DEFAULT_COLLECTION_SIZE)),
        ecalSimHits_(new TClonesArray(event::SIM_CALORIMETER_HIT, event::DEFAULT_COLLECTION_SIZE)),
        hcalSimHits_(new TClonesArray(event::SIM_CALORIMETER_HIT, event::DEFAULT_COLLECTION_SIZE)),
        targetSimHits_(new TClonesArray(event::SIM_CALORIMETER_HIT, event::DEFAULT_COLLECTION_SIZE)),
        triggerPadSimHits_(new TClonesArray(event::SIM_CALORIMETER_HIT, event::DEFAULT_COLLECTION_SIZE)) {

    collMap_[RECOIL_SIM_HITS] = recoilSimHits_;
    collMap_[TAGGER_SIM_HITS] = taggerSimHits_;
    collMap_[SIM_PARTICLES] = simParticles_;
    collMap_[ECAL_SIM_HITS] = ecalSimHits_;
    collMap_[HCAL_SIM_HITS] = hcalSimHits_;
    collMap_[TRIGGER_PAD_SIM_HITS] = triggerPadSimHits_;
    collMap_[TARGET_SIM_HITS] = targetSimHits_;
}

}

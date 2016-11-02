#include "Event/SimEvent.h"

ClassImp(event::SimEvent)

namespace event {

SimEvent::SimEvent() :
        simParticles(new TClonesArray(event::SIM_PARTICLE, event::DEFAULT_COLLECTION_SIZE)),
        taggerSimHits(new TClonesArray(event::SIM_TRACKER_HIT, event::DEFAULT_COLLECTION_SIZE)),
        recoilSimHits(new TClonesArray(event::SIM_TRACKER_HIT, event::DEFAULT_COLLECTION_SIZE)),
        ecalSimHits(new TClonesArray(event::SIM_CALORIMETER_HIT, event::DEFAULT_COLLECTION_SIZE)),
        hcalSimHits(new TClonesArray(event::SIM_CALORIMETER_HIT, event::DEFAULT_COLLECTION_SIZE)),
        targetSimHits(new TClonesArray(event::SIM_CALORIMETER_HIT, event::DEFAULT_COLLECTION_SIZE)),
        triggerPadSimHits(new TClonesArray(event::SIM_CALORIMETER_HIT, event::DEFAULT_COLLECTION_SIZE)) {

    collMap[RECOIL_SIM_HITS] = recoilSimHits;
    collMap[TAGGER_SIM_HITS] = taggerSimHits;
    collMap[SIM_PARTICLES] = simParticles;
    collMap[ECAL_SIM_HITS] = ecalSimHits;
    collMap[HCAL_SIM_HITS] = hcalSimHits;
    collMap[TRIGGER_PAD_SIM_HITS] = triggerPadSimHits;
    collMap[TARGET_SIM_HITS] = targetSimHits;
}

}

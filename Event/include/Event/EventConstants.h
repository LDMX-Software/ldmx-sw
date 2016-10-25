#ifndef Event_EventConstants_h
#define Event_EventConstants_h

namespace event {

    /*
     * Type names.
     */
    const static char* SIM_EVENT = "event::SimEvent";
    const static char* SIM_PARTICLE = "event::SimParticle";
    const static char* SIM_CALORIMETER_HIT = "event::SimCalorimeterHit";
    const static char* SIM_TRACKER_HIT = "event::SimTrackerHit";

    /*
     * Collection names.
     */
    const static char* SIM_PARTICLES = "SimParticles";
    const static char* RECOIL_SIM_HITS = "RecoilSimHits";
    const static char* TAGGER_SIM_HITS = "TaggerSimHits";
    const static char* ECAL_SIM_HITS = "EcalSimHits";
    const static char* HCAL_SIM_HITS = "HcalSimHits";

    /*
     * Default collection size for TClonesArray in event objects.
     */
    const static int DEFAULT_COLLECTION_SIZE = 100;
}

#endif

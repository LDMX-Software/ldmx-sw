#ifndef EVENT_EVENTCONSTANTS_H_
#define EVENT_EVENTCONSTANTS_H_

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
    const static char* TRIGGER_PAD_SIM_HITS = "TriggerPadSimHits";
    const static char* TARGET_SIM_HITS = "TargetSimHits";

    /*
     * Default collection size for TClonesArray in event objects.
     */
    const static int DEFAULT_COLLECTION_SIZE = 100;
}

#endif

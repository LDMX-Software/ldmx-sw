#ifndef EVENT_EVENTCONSTANTS_H_
#define EVENT_EVENTCONSTANTS_H_

namespace event {

class EventConstants {

    public:

        /*
         * Type names.
         */
        static constexpr char* SIM_EVENT = (char*) "event::SimEvent";
        static constexpr char* SIM_PARTICLE = (char*) "event::SimParticle";
        static constexpr char* SIM_CALORIMETER_HIT = (char*) "event::SimCalorimeterHit";
        static constexpr char* SIM_TRACKER_HIT = (char*) "event::SimTrackerHit";

        /*
         * Collection names.
         */
        static constexpr char* SIM_PARTICLES = (char*) "SimParticles";
        static constexpr char* RECOIL_SIM_HITS = (char*) "RecoilSimHits";
        static constexpr char* TAGGER_SIM_HITS = (char*) "TaggerSimHits";
        static constexpr char* ECAL_SIM_HITS = (char*) "EcalSimHits";
        static constexpr char* HCAL_SIM_HITS = (char*) "HcalSimHits";
        static constexpr char* TRIGGER_PAD_SIM_HITS = (char*) "TriggerPadSimHits";
        static constexpr char* TARGET_SIM_HITS = (char*) "TargetSimHits";

        /*
         * Default collection size for TClonesArray in event objects.
         */
        const static int DEFAULT_COLLECTION_SIZE = 1000;

}; // class EventConstants

} // namespace event

#endif

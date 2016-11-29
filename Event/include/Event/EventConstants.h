#ifndef EVENT_EVENTCONSTANTS_H_
#define EVENT_EVENTCONSTANTS_H_

#include <string>

namespace event {

class EventConstants {

    public:

        /*
         * Type names.
         */
        static const std::string SIM_EVENT;
        static const std::string SIM_PARTICLE;
        static const std::string SIM_CALORIMETER_HIT;
        static const std::string SIM_TRACKER_HIT;

        /*
         * Collection names.
         */
        static const std::string SIM_PARTICLES;
        static const std::string RECOIL_SIM_HITS;
        static const std::string TAGGER_SIM_HITS;
        static const std::string ECAL_SIM_HITS;
        static const std::string HCAL_SIM_HITS;
        static const std::string TRIGGER_PAD_SIM_HITS;
        static const std::string TARGET_SIM_HITS;

        /*
         * Default collection size for TClonesArray in event objects.
         */
        const static int DEFAULT_COLLECTION_SIZE = 1000;

}; // class EventConstants

} // namespace event

#endif

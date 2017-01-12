/**
 * @file EventConstants.h
 * @brief Class providing string constants for the event model
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef EVENT_EVENTCONSTANTS_H_
#define EVENT_EVENTCONSTANTS_H_

#include <string>

namespace event {

/**
 * @class EventConstants
 * @brief Provides access to static event constants used by the Event class
 */
class EventConstants {

    public:

        /*
         * Type names.
         */
        static const std::string RUN_HEADER;
        static const std::string SIM_CALORIMETER_HIT;
        static const std::string SIM_EVENT;
        static const std::string SIM_PARTICLE;
        static const std::string SIM_TRACKER_HIT;

        /*
         * Collection names.
         */
        static const std::string ECAL_SIM_HITS;
        static const std::string HCAL_SIM_HITS;
        static const std::string RECOIL_SIM_HITS;
        static const std::string SIM_PARTICLES;
        static const std::string TAGGER_SIM_HITS;
        static const std::string TARGET_SIM_HITS;
        static const std::string TRIGGER_PAD_SIM_HITS;

        static const std::string EVENT_TREE_NAME;

        /*
         * Default collection size for TClonesArray in event objects.
         */
        const static int DEFAULT_COLLECTION_SIZE = 1000;

}; // class EventConstants

} // namespace event

#endif

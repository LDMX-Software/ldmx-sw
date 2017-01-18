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

        /**
         * Default name of event tree.
         */
        static const std::string EVENT_TREE_NAME;

        /*
         * Default collection and object names in the event tree.
         */
        static const std::string ECAL_SIM_HITS;
        static const std::string EVENT_HEADER;
        static const std::string HCAL_SIM_HITS;
        static const std::string RECOIL_SIM_HITS;
        static const std::string SIM_PARTICLES;
        static const std::string TAGGER_SIM_HITS;
        static const std::string TARGET_SIM_HITS;
        static const std::string TRIGGER_PAD_SIM_HITS;
        static const std::string TRIGGER_RESULT;

}; // class EventConstants

} // namespace event

#endif

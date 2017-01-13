#include "Event/EventConstants.h"

namespace event {

    /*
     * Default name of event tree.
     */
    const std::string EventConstants::EVENT_TREE_NAME = "LDMX_Events";

    /*
     * Default Collection and object names in the event tree.
     */
    const std::string EventConstants::ECAL_SIM_HITS = "EcalSimHits";
    const std::string EventConstants::EVENT_HEADER = "EventHeader";
    const std::string EventConstants::HCAL_SIM_HITS = "HcalSimHits";
    const std::string EventConstants::RECOIL_SIM_HITS = "RecoilSimHits";
    const std::string EventConstants::SIM_PARTICLES = "SimParticles";
    const std::string EventConstants::TAGGER_SIM_HITS = "TaggerSimHits";
    const std::string EventConstants::TARGET_SIM_HITS = "TargetSimHits";
    const std::string EventConstants::TRIGGER_PAD_SIM_HITS = "TriggerPadSimHits";
    const std::string EventConstants::TRIGGER_RESULT = "TriggerResult";

} // namespace event

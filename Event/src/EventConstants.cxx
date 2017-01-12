#include "Event/EventConstants.h"

namespace event {

    /*
     * Type names.
     */
    const std::string EventConstants::RUN_HEADER = "event::RunHeader";
    const std::string EventConstants::SIM_CALORIMETER_HIT = "event::SimCalorimeterHit";
    const std::string EventConstants::SIM_EVENT = "event::SimEvent";
    const std::string EventConstants::SIM_PARTICLE = "event::SimParticle";
    const std::string EventConstants::SIM_TRACKER_HIT = "event::SimTrackerHit";

    /*
     * Collection names.
     */
    const std::string EventConstants::ECAL_SIM_HITS = "EcalSimHits";
    const std::string EventConstants::HCAL_SIM_HITS = "HcalSimHits";
    const std::string EventConstants::RECOIL_SIM_HITS = "RecoilSimHits";
    const std::string EventConstants::SIM_PARTICLES = "SimParticles";
    const std::string EventConstants::TAGGER_SIM_HITS = "TaggerSimHits";
    const std::string EventConstants::TARGET_SIM_HITS = "TargetSimHits";
    const std::string EventConstants::TRIGGER_PAD_SIM_HITS = "TriggerPadSimHits";

    const std::string EventConstants::EVENT_TREE_NAME = "LDMX_Events";

} // namespace event

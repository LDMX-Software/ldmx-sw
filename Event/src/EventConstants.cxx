#include "Event/EventConstants.h"

namespace event {

    const std::string EventConstants::SIM_EVENT = "event::SimEvent";
    const std::string EventConstants::SIM_PARTICLE = "event::SimParticle";
    const std::string EventConstants::SIM_CALORIMETER_HIT = "event::SimCalorimeterHit";
    const std::string EventConstants::SIM_TRACKER_HIT = "event::SimTrackerHit";

    const std::string EventConstants::SIM_PARTICLES = "SimParticles";
    const std::string EventConstants::RECOIL_SIM_HITS = "RecoilSimHits";
    const std::string EventConstants::TAGGER_SIM_HITS = "TaggerSimHits";
    const std::string EventConstants::ECAL_SIM_HITS = "EcalSimHits";
    const std::string EventConstants::HCAL_SIM_HITS = "HcalSimHits";
    const std::string EventConstants::TRIGGER_PAD_SIM_HITS = "TriggerPadSimHits";
    const std::string EventConstants::TARGET_SIM_HITS = "TargetSimHits";

    const double EventConstants::ECAL_MAP_XY = 1000;
    const double EventConstants::CELL_SIZE = 4.59360;
} // namespace event

#include "Recon/Event/EventConstants.h"

namespace ldmx {

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
const std::string EventConstants::CLUSTER_ALGO_RESULT = "ClusterAlgoResult";

/*
 * Type names.
 */
const std::string EventConstants::ECAL_HIT = "ldmx::EcalHit";
const std::string EventConstants::ECAL_CLUSTER = "ldmx::EcalCluster";
const std::string EventConstants::HCAL_HIT = "ldmx::HcalHit";
const std::string EventConstants::SIM_PARTICLE = "ldmx::SimParticle";
const std::string EventConstants::SIM_CALORIMETER_HIT =
    "ldmx::SimCalorimeterHit";
const std::string EventConstants::SIM_TRACKER_HIT = "ldmx::SimTrackerHit";
const std::string EventConstants::RUN_HEADER = "ldmx::RunHeader";
const std::string EventConstants::PN_WEIGHT = "simcore::pnWeight";

}  // namespace ldmx

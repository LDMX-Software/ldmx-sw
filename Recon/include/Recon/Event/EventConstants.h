/**
 * @file EventConstants.h
 * @brief Class providing string constants for the event model
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef RECON_EVENT_EVENTCONSTANTS_H_
#define RECON_EVENT_EVENTCONSTANTS_H_

#include <string>

namespace ldmx {

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
  static const std::string CLUSTER_ALGO_RESULT;

  /*
   * Type names, mostly for initializing clones arrays.
   */
  static const std::string ECAL_HIT;
  static const std::string ECAL_CLUSTER;
  static const std::string HCAL_HIT;
  static const std::string SIM_PARTICLE;
  static const std::string SIM_CALORIMETER_HIT;
  static const std::string SIM_TRACKER_HIT;
  static const std::string RUN_HEADER;
  static const std::string PN_WEIGHT;

};  // class EventConstants
}  // namespace ldmx

#endif

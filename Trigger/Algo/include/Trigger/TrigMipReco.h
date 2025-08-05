/**
 * @file TrigMipReco.h
 * @brief Trigger Calo MIP finding algorithm
 * @author Christian Herwig, Michigan
 */

#ifndef TRIGGER_TRIGMIPRECO_H
#define TRIGGER_TRIGMIPRECO_H

// LDMX Framework
#include <chrono>

#include "DetDescr/HcalTriggerID.h"
#include "Framework/Configure/Parameters.h"
#include "Framework/Event.h"
#include "Framework/EventProcessor.h"
#include "Trigger/Event/TrigCaloHit.h"
#include "Trigger/Event/TrigMip.h"

namespace trigger {

/**
 * @class TrigMipReco
 * @brief
 */
class TrigMipReco : public framework::Producer {
 public:
  TrigMipReco(const std::string& name, framework::Process& process)
      : framework::Producer(name, process) {}

  void configure(framework::config::Parameters& ps) override;

  void produce(framework::Event& event) override;

  /**
   * onNewRun is the first function called for each processor
   * *after* the conditions are fully configured and accessible.
   * This is where you could create single-processors, multi-event
   * calculation objects.
   */
  void onNewRun(const ldmx::RunHeader& rh) override;

  /**
   *
   */
  void onProcessEnd() override;

 private:
  int nevents_{0};
  double processing_time_{0.};
  std::map<std::string, double> profiling_map_;
  // name of collection for Hcal TP hits_ to be passed as input
  std::string hit_coll_name_;
  std::string hit_coll_passname_;
  // name of output collection
  std::string pass_coll_name_;
  // calorimeter_type_is_hcal_ boolean
  bool calorimeter_type_is_hcal_;

  int max_layer_;
  int min_track_length_;
  // MIP peak is 10-11 MeV usually
  float hcal_min_energy_;
  // MIP peak around 17 MeV usually
  float ecal_min_energy_;
  // MeV
  float ecal_max_energy_;
  // mm
  float search_radius_;
  // MeV; Change as needed
  float isolation_e_cut_;
  // No more than 20% hole fraction ; Change as needed ; Converts tracks to MIP
  // objects
  float hole_fraction_max_;
};
}  // namespace trigger

#endif /* HCALTPSELECTOR_H */

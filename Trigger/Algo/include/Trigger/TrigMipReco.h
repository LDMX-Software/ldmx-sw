/**
 * @file TrigMipReco.h
 * @brief Trigger Calo MIP finding algorithm
 * @author Christian Herwig, Michigan
 */

#ifndef TRIGGER_TRIGMIPRECO_H
#define TRIGGER_TRIGMIPRECO_H

// LDMX Framework
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

  virtual void configure(framework::config::Parameters& ps);

  virtual void produce(framework::Event& event);

 private:
  // name of collection for Hcal TP hits to be passed as input
  std::string hit_coll_name_;
  std::string hit_coll_passname_;
  // name of output collection
  std::string pass_coll_name_;
  // calorimeter_type_is_hcal_ boolean
  bool calorimeter_type_is_hcal_;

  float min_energy_;  // set in configure()
  float max_energy_;  // set in configure()
};
}  // namespace trigger

#endif /* HCALTPSELECTOR_H */

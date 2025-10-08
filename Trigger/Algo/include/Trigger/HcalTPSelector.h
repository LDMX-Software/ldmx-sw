/**
 * @file HcalTPSelector.h
 * @brief Hcal clustering algorithm
 * @author Christian Herwig, Michigan
 */

#ifndef TRIGGER_HCALTPSELECTOR_H
#define TRIGGER_HCALTPSELECTOR_H

// LDMX Framework
#include "DetDescr/HcalTriggerID.h"
#include "Framework/Configure/Parameters.h"  // Needed to import parameters from configuration file
#include "Framework/Event.h"
#include "Framework/EventProcessor.h"  //Needed to declare processor
#include "Recon/Event/CaloTrigPrim.h"
#include "TrigUtilities.h"
#include "Trigger/Event/TrigCaloHit.h"
#include "Trigger/Event/TrigEnergySum.h"

namespace trigger {

/**
 * @class HcalTPSelector
 * @brief
 */
class HcalTPSelector : public framework::Producer {
 public:
  HcalTPSelector(const std::string& name, framework::Process& process)
      : framework::Producer(name, process) {}

  virtual void configure(framework::config::Parameters& ps);

  virtual void produce(framework::Event& event);

 private:
  // name of collection for HcalTPs to be passed as input
  std::string combined_quad_coll_name_;
  // name of output collection
  std::string pass_coll_name_;

  std::string tp_coll_passname_;

  std::string tp_coll_event_passname_;
};
}  // namespace trigger

#endif /* HCALTPSELECTOR_H */

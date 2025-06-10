/**
 * @file TrigMipReco.h
 * @brief Trigger Calo MIP finding algorithm
 * @author Christian Herwig, Michigan
 */

#ifndef HCALTPSELECTOR_H
#define HCALTPSELECTOR_H

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
  std::string hitCollName_;
  // name of output collection
  std::string passCollName_;
  // calorimeterTypeIsHcal boolean
  bool calorimeterTypeIsHcal_;

  float minEnergy_;  // set in configure()
  float maxEnergy_;  // set in configure()
};
}  // namespace trigger

#endif /* HCALTPSELECTOR_H */

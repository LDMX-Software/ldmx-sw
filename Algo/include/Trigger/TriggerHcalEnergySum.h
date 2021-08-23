/**
 * @file TriggerHcalEnergySum.h
 * @brief HcalEnergySum algo
 * @author Christian Herwig, Fermilab
 */

#ifndef TRIGGERHCALENERGYSUM_H
#define TRIGGERHCALENERGYSUM_H

// LDMX Framework
// #include "Hcal/HcalTriggerGeometry.h"
#include "Framework/Configure/Parameters.h"  // Needed to import parameters from configuration file
#include "Framework/Event.h"
#include "Framework/EventProcessor.h"  //Needed to declare processor
#include "ap_fixed.h"
#include "ap_int.h"

namespace trigger {

/**
 * @class TriggerHcalEnergySum
 * @brief
 */
class TriggerHcalEnergySum : public framework::Producer {
 public:
  TriggerHcalEnergySum(const std::string& name, framework::Process& process)
      : framework::Producer(name, process) {}

  virtual void configure(framework::config::Parameters& ps);

  virtual void produce(framework::Event& event);

  virtual void onFileOpen();

  virtual void onFileClose();

  virtual void onProcessStart();

  virtual void onProcessEnd();

  typedef ap_ufixed<16, 14> e_t;  // [MeV] (Up to at least 8 GeV)

 private:
  // specific verbosity of this producer
  int verbose_{0};

  // ClusterGeometry myGeo;
};
}  // namespace trigger

#endif /* TRIGGERECALENERGYSUM_H */

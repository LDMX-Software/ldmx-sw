/**
 * @file HcalTPSelector.h
 * @brief Hcal clustering algorithm
 * @author Christian Herwig, Michigan
 */

#ifndef HCALTPSELECTOR_H
#define HCALTPSELECTOR_H

// LDMX Framework
// #include "DetDescr/HcalGeometry.h"
// #include "Hcal/HcalTriggerGeometry.h"
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

  // helpers
  // void decodeTP(ldmx::CaloTrigPrim tp, double& x, double& y, double& z,
  //               double& e);
  /* double primitiveToEnergy(int tp, int layer); */

 private:
  // name of collection for HcalTPs to be passed as input
  std::string combinedQuadCollName_;
  // name of output collection
  std::string passCollName_;

  // unsigned int maxCentralTPs_{12};
  // unsigned int maxOuterTPs_{8};
};
}  // namespace trigger

#endif /* HCALTPSELECTOR_H */

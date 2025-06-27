/**
 * @file PFTruthProducer.h
 * @brief Track selection skeleton for PFlow Reco
 * @author Christian Herwig, Fermilab
 */

#ifndef PFTRUTHPRODUCER_H
#define PFTRUTHPRODUCER_H

// LDMX Framework
#include "Framework/Configure/Parameters.h"  // Needed to import parameters from configuration file
#include "Framework/Event.h"
#include "Framework/EventProcessor.h"  //Needed to declare processor

namespace recon {

/**
 * @class PFTruthProducer
 * @brief
 */
class PFTruthProducerBrem : public framework::Producer {
 public:
  PFTruthProducerBrem(const std::string& name, framework::Process& process)
      : framework::Producer(name, process) {}

  virtual void configure(framework::config::Parameters& ps);

  virtual void produce(framework::Event& event);

 private:
  // name of collection for target, ecal, hcal truth to be output
  std::string primaryCollName_;
  std::string targetCollName_;
  std::string ecalCollName_;
  std::string hcalCollName_;

  std::string outputCollName_;

};
}  // namespace recon

#endif /* PFTRUTHPRODUCER_H */

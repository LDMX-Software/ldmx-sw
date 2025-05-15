/**
 * @file PFTruthProducer.h
 * @brief Track selection skeleton for PFlow Reco
 * @author Christian Herwig, Fermilab
 */

#ifndef PFTRACKPRODUCER_H
#define PFTRACKPRODUCER_H

// LDMX Framework
#include "Framework/Configure/Parameters.h"  // Needed to import parameters from configuration file
#include "Framework/Event.h"
#include "Framework/EventProcessor.h"  //Needed to declare processor

namespace recon {

/**
 * @class PFTruthProducer
 * @brief
 */
class PFTruthProducer : public framework::Producer {
 public:
  PFTruthProducer(const std::string& name, framework::Process& process)
      : framework::Producer(name, process) {}

  virtual void configure(framework::config::Parameters& ps);
  //void configure(framework::config::Parameters& parameters) override;

  virtual void produce(framework::Event& event);

 private:
  // name of collection for target, ecal, hcal truth to be output
  std::string primaryCollName_;
  std::string targetCollName_;
  std::string ecalCollName_;
  std::string hcalCollName_;
  std::string target_sp_passname_;
  std::string ecal_sp_passname_;
  std::string sim_particles_passname_;
  std::string target_sp_hits_event_passname_;
  std::string ecal_sp_hits_event_passname_;
  std::string sim_particles_event_passname_;
};
}  // namespace recon

#endif /* PFTRACKPRODUCER_H */

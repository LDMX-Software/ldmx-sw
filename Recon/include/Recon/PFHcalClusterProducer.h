/**
 * @file PFHcalClusterProducer.h
 * @brief HCal clustering skeleton for PFlow Reco
 * @author Christian Herwig, Fermilab
 */

#ifndef PFHCALCLUSTERPRODUCER_H
#define PFHCALCLUSTERPRODUCER_H

// LDMX Framework
#include "Framework/Configure/Parameters.h"  // Needed to import parameters from configuration file
#include "Framework/Event.h"
#include "Framework/EventProcessor.h"  //Needed to declare processor

namespace recon {

/**
 * @class PFHcalClusterProducer
 * @brief
 */
class PFHcalClusterProducer : public framework::Producer {
 public:
  PFHcalClusterProducer(const std::string& name, framework::Process& process)
      : framework::Producer(name, process) {}

  virtual void configure(framework::config::Parameters& ps);

  virtual void produce(framework::Event& event);

 private:
  bool singleCluster_{true};
  bool logEnergyWeight_{true};

  float minHitEnergy_{0};
  float clusterHitDist_{100.};
  float clusterZBias_{1.};  // private parameter for z bias
  int minClusterHitMult_{2};

  // name of collection for hits to be passed as input
  std::string hitCollName_;
  // name of collection for pfCluster to be output
  std::string clusterCollName_;
  std::string suffix_;
};
}  // namespace recon

#endif /* PFHCALCLUSTERPRODUCER_H */

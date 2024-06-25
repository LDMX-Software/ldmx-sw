/**
 * @file PFEcalClusterProducer.h
 * @brief ECal clustering skeleton for PFlow Reco
 * @author Christian Herwig, Fermilab
 */

#ifndef PFECALCLUSTERPRODUCER_H
#define PFECALCLUSTERPRODUCER_H

// LDMX Framework
#include "DetDescr/EcalGeometry.h"
#include "Ecal/Event/EcalCluster.h"
#include "Ecal/Event/EcalHit.h"
#include "Ecal/MyClusterWeight.h"
#include "Ecal/TemplatedClusterFinder.h"
#include "Ecal/WorkingCluster.h"
#include "Framework/Configure/Parameters.h"  // Needed to import parameters from configuration file
#include "Framework/Event.h"
#include "Framework/EventProcessor.h"  //Needed to declare processor
#include "TFitResult.h"
#include "TGraph.h"

namespace recon {

/**
 * @class PFEcalClusterProducer
 * @brief
 */
class PFEcalClusterProducer : public framework::Producer {
 public:
  PFEcalClusterProducer(const std::string& name, framework::Process& process)
      : framework::Producer(name, process) {}

  virtual void configure(framework::config::Parameters& ps);

  virtual void produce(framework::Event& event);

  virtual void onFileOpen();

  virtual void onFileClose();

  virtual void onProcessStart();

  virtual void onProcessEnd();

 private:
  // specific verbosity of this producer
  int verbose_{0};
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

#endif /* PFECALCLUSTERPRODUCER_H */

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

 private:
  bool single_cluster_{true};
  bool log_energy_weight_{true};
  bool save_hit_contribs_{true};

  float min_hit_energy_{0};
  float cluster_hit_dist_{100.};
  float cluster_z_bias_{1.};  // private parameter for z_ bias
  int min_cluster_hit_mult_{2};

  // name of collection for hits to be passed as input
  std::string hit_coll_name_;
  // pass name of hit collection to be passed as input
  std::string hit_pass_name_;
  // name of collection for pfCluster to be output
  std::string cluster_coll_name_;
  std::string suffix_;
};
}  // namespace recon

#endif /* PFECALCLUSTERPRODUCER_H */

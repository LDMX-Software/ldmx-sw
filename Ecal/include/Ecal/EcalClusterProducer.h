/**
 * @file EcalClusterProducer.h
 * @brief Simple algorithm that does clustering in the ECal
 * @author Josh Hiltbrand, University of Minnesota
 */

#ifndef ECAL_ECALCLUSTERPRODUCER_H_
#define ECAL_ECALCLUSTERPRODUCER_H_

//----------//
//   ROOT   //
//----------//
#include "TH1F.h"
#include "TH2F.h"

//----------//
//   LDMX   //
//----------//
#include "DetDescr/DetectorID.h"
#include "DetDescr/EcalGeometry.h"
#include "DetDescr/EcalID.h"
#include "Ecal/CLUE.h"
#include "Ecal/Event/ClusterAlgoResult.h"
#include "Ecal/Event/EcalCluster.h"
#include "Ecal/Event/EcalHit.h"
#include "Ecal/IntermediateCluster.h"
#include "Ecal/MyClusterWeight.h"
#include "Ecal/TemplatedClusterFinder.h"
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"

//----------//
//    STL   //
//----------//
#include <memory>
#include <tuple>

namespace ecal {

/**
 * @class EcalClusterProducer
 * @brief Simple algorithm that does clustering in the ECal
 */
class EcalClusterProducer : public framework::Producer {
 public:
  EcalClusterProducer(const std::string& name, framework::Process& process);
  ~EcalClusterProducer() override = default;

  /**
   * Configure the processor using the given user specified parameters.
   *
   * @param parameters Set of parameters used to configure this processor.
   */
  void configure(framework::config::Parameters& parameters) override;

  void produce(framework::Event& event) override;

 private:
  double seed_threshold_{0};
  double cutoff_{0};

  double dc_{0};
  double rhoc_{0};
  double deltac_{0};
  double deltao_{0};
  // cutoff for log-e weighting in RMS calculation
  float minHitEnergy_{1.}; 

  std::string rec_hit_coll_name_;
  std::string rec_hit_pass_name_;
  std::string algo_coll_name_;
  std::string cluster_coll_name_;

  bool CLUE_;
  int nbr_of_layers_;
  bool reclustering_;

  /** The name of the cluster algorithm used. */
  TString algo_name_;
};
}  // namespace ecal

#endif

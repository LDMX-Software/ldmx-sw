/**
 * @file EcalClusterAnalyzer.h
 * @brief Analysis of cluster performance
 * @author Ella Viirola, Lund University
 * @author Tamas Almos Vami, UCSB
 */

#ifndef DQM_ECALCLUSTERANALYZER_H
#define DQM_ECALCLUSTERANALYZER_H

#include <algorithm>
#include <fstream>
#include <iostream>

// LDMX Framework
#include "DetDescr/SimSpecialID.h"
#include "Ecal/Event/EcalCluster.h"
#include "Ecal/Event/EcalHit.h"
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"
#include "SimCore/Event/SimCalorimeterHit.h"
#include "SimCore/Event/SimTrackerHit.h"

namespace dqm {

/**
 * @class EcalClusterAnalyzer
 * @brief
 */
class EcalClusterAnalyzer : public framework::Analyzer {
 public:
  EcalClusterAnalyzer(const std::string& name, framework::Process& process)
      : Analyzer(name, process) {}
  ~EcalClusterAnalyzer() override = default;
  void configure(framework::config::Parameters& ps) override;
  void analyze(const framework::Event& event) override;

 private:
  /// Use the number of simulated electrons
  /// instead of the number of determined by the TS track counting
  bool use_simulated_electron_number_;

  /// What is the number of electrons in the event?
  int nbr_of_electrons_;

  // Collection Name for SimHits
  std::string ecal_sim_hit_coll_;

  // Pass Name for SimHits
  std::string ecal_sim_hit_pass_;

  // Collection Name for RecHits
  std::string rec_hit_coll_name_;

  // Pass Name for RecHits
  std::string rec_hit_pass_name_;

  // Collection Name for clusters
  std::string cluster_coll_name_;

  // Pass Name for clusters
  std::string cluster_pass_name_;

  // Collection Name for Scoring Plane hits
  std::string ecal_sp_hits_coll_name_;
  // Pass Name for Scoring Plane hits
  std::string ecal_sp_hits_pass_name_;

  // min energy fraction from smaller contributor to consider hit "mixed"
  double mixed_hit_cutoff_;
  std::string ecal_sp_hits_passname_;

  // Inverse skim flag
  bool inverse_skim_;

  // Minimum number of ECal clusters to keep the event
  int n_ecal_clusters_min_;
};

}  // namespace dqm

#endif

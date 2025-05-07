/**
 * @file EcalClusterAnalyzer.h
 * @brief Analysis of cluster performance
 * @author Ella Viirola, Lund University
 */

#ifndef DQM_ECALCLUSTERANALYZER_H
#define DQM_ECALCLUSTERANALYZER_H

// LDMX Framework
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"

namespace ecal {

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
};

}  // namespace ecal

#endif

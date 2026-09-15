#pragma once

#include <string>

#include "Framework/Event.h"
#include "Framework/EventProcessor.h"

namespace tracking::dqm {

/**
 * DQM analyzer for silicon-strip digitization and clustering.
 *
 * Per-sensor histograms are produced for:
 *
 *   Sim hits (from SimTrackerHit collection)
 *     sim_n_sN        — number of sim hits per event
 *     sim_edep_sN     — energy deposition [MeV]
 *
 *   Fitted strips (from FittedSiStripHit collection)
 *     strip_n_sN      — number of fitted strips per event
 *     strip_amp_sN    — fitted amplitude [ADC counts]
 *     strip_t0_sN     — fitted hit time [ns]
 *     strip_chi2ndf_sN — chi2/ndf of the pulse fit
 *
 *   Clusters (from StripClusterProcessor Measurement collection)
 *     cluster_n_sN    — number of clusters per event
 *     cluster_u_sN    — cluster centroid local U [mm]
 *     cluster_amp_sN  — total cluster amplitude [ADC counts]
 *     cluster_nstrips_sN — number of strips per cluster
 *     cluster_time_sN — cluster hit time [ns]
 *     sim_cluster_du_sN — truth U minus cluster U [mm]  (requires
 * digi_coll_name)
 *
 * Configuration parameters
 * ------------------------
 *   sim_coll_name     SimTrackerHit collection  (default "TaggerSimHits")
 *   sim_pass_name     pass name                 (default "")
 *   digi_coll_name    digi Measurement coll. for truth_u lookup (default "" =
 * disabled) digi_pass_name    pass name                 (default "")
 *   fitted_coll_name  FittedSiStripHit coll.    (default
 * "TaggerFittedSiStripHits") fitted_pass_name  pass name (default "")
 *   cluster_coll_name cluster Measurement coll. (default
 * "TaggerClusterMeasurements") cluster_pass_name pass name (default "")
 *   n_sensors         number of sensors         (default 14)
 */
class DigiDQM : public framework::Analyzer {
 public:
  DigiDQM(const std::string& name, framework::Process& process)
      : framework::Analyzer(name, process) {}

  ~DigiDQM() = default;

  void configure(framework::config::Parameters& parameters) override;
  void analyze(const framework::Event& event) override;

 private:
  std::string sim_coll_name_{"TaggerSimHits"};
  std::string sim_pass_name_{""};
  std::string digi_coll_name_{""};
  std::string digi_pass_name_{""};
  std::string fitted_coll_name_{"TaggerFittedSiStripHits"};
  std::string fitted_pass_name_{""};
  std::string cluster_coll_name_{"TaggerClusterMeasurements"};
  std::string cluster_pass_name_{""};
  int n_sensors_{14};
};

}  // namespace tracking::dqm

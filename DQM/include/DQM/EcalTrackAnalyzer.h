/**
 * @file EcalTrackAnalyzer.h
 * @brief Analysis of ECAL tracks produced by ACTS track finder
 * @author Tamas Almos Vami (UCSB)  
 */

#ifndef DQM_ECALTRACKANALYZER_H
#define DQM_ECALTRACKANALYZER_H

#include <string>

// LDMX Framework
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"
#include "Tracking/Event/Track.h"

namespace dqm {

/**
 * @class EcalTrackAnalyzer
 * @brief DQM analyzer for ECAL tracks fitted with ACTS zero-B field CKF
 *
 * Creates histograms of track properties:
 * - Track multiplicity
 * - Number of hits per track
 * - Chi2 and NDF distributions
 * - Track parameters (d0, z0, phi, theta, p)
 * - Hit occupancy by layer
 */
class EcalTrackAnalyzer : public framework::Analyzer {
 public:
  /**
   * Constructor
   */
  EcalTrackAnalyzer(const std::string& name, framework::Process& process)
      : Analyzer(name, process) {}

  /**
   * Destructor
   */
  ~EcalTrackAnalyzer() override = default;

  /**
   * Configure the analyzer
   */
  void configure(framework::config::Parameters& ps) override;

  /**
   * Analyze the event
   */
  void analyze(const framework::Event& event) override;

 private:
  /// ECAL track collection name
  std::string track_collection_{"EcalTracks"};

  /// Pass name for tracks
  std::string track_pass_name_{""};

  /// ECAL RecHit collection (for layer occupancy)
  std::string rec_hit_collection_{"EcalRecHits"};

  /// Pass name for RecHits
  std::string rec_hit_pass_name_{""};
};

}  // namespace dqm

#endif  // DQM_ECALTRACKANALYZER_H

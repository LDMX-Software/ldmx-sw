#pragma once

#include <algorithm>
#include <iostream>

#include "Framework/Configure/Parameters.h"
#include "Framework/Event.h"
#include "Framework/EventProcessor.h"
#include "SimCore/Event/SimTrackerHit.h"
#include "Tracking/Event/Measurement.h"
#include "Tracking/Event/Track.h"
#include "Tracking/Event/Track.h"
#include "Tracking/Event/TruthTrack.h"
#include "Tracking/Sim/TrackingUtils.h"

namespace tracking::dqm {

enum PIDBins {
  kminus = -4,
  antiproton,
  piminus,
  positron,
  electron,
  piplus,
  proton,
  kplus
};

class TrackingRecoDQM : public framework::Analyzer {
 public:
  TrackingRecoDQM(const std::string& name, framework::Process& process)
      : framework::Analyzer(name, process){};

  /// Destructor
  ~TrackingRecoDQM() = default;

  void analyze(const framework::Event& event) override;

  void trackMonitoring(const std::vector<ldmx::Track>& tracks,
                       const std::vector<ldmx::Measurement>& measurements,
                       const std::string title, const bool& doDetail,
                       const bool& doTruth);

  void efficiencyPlots(const std::vector<ldmx::Track>& tracks,
                       const std::vector<ldmx::Measurement>& measurements,
                       const std::string& title);

  /** Monitoring plots for tracks extrapolated to the ECAL Scoring plane.
   *  This aims
   *  Tracks will be truth matched first to get the trackID. The hit with the
   * trackID
   *
   */

  void trackStateMonitoring(const std::vector<ldmx::Track>& tracks,
                            ldmx::TrackStateType ts_type,
                            const std::string& ts_title);
  /**
   * Configure the analyzer using the given user specified parameters.
   *
   * @param parameters Set of parameters used to configure this analyzer.
   */

  void configure(framework::config::Parameters& parameters) override;

  void onProcessEnd() override;

  void sortTracks(const std::vector<ldmx::Track>& tracks,
                  std::vector<ldmx::Track>& uniqueTracks,
                  std::vector<ldmx::Track>& duplicateTracks,
                  std::vector<ldmx::Track>& fakeTracks);

 private:
  std::string track_collection_;
  std::string truth_collection_;
  std::string measurement_collection_;
  std::string measurement_passname_;

  std::string ecal_sp_events_passname_;
  std::string ecal_sp_passname_;
  std::string target_sp_events_passname_;
  std::string target_sp_passname_;
  std::string track_collection_events_passname_;
  std::string track_passname_;
  std::string truth_events_passname_;
  std::string truth_passname_;

  std::string title_{"tagger_trk_"};
  double track_prob_cut_{0.5};
  std::string subdetector_{"Tagger"};
  bool do_truth_comparison_{false};
  std::vector<std::string> track_states_;

  // Truth Track collection
  std::shared_ptr<std::vector<ldmx::Track>> truth_track_collection_{nullptr};

  // Ecal scoring plane hits_
  std::shared_ptr<std::vector<ldmx::SimTrackerHit>> ecal_scoring_hits_{nullptr};

  // Target  scoring plane hits_
  std::shared_ptr<std::vector<ldmx::SimTrackerHit>> target_scoring_hits_{
      nullptr};

  // If I have truth information, sort the tracks vector according to their
  // trackID and truthProb
  // real tracks (truth_prob > cut), unique
  std::vector<ldmx::Track> unique_tracks_;
  // real tracks (truth_prob > cut), duplicated
  std::vector<ldmx::Track> duplicate_tracks_;
  // fake tracks (truth_prob < cut)
  std::vector<ldmx::Track> fake_tracks_;

  // PID mapping
  std::map<int, int> pidmap_;
};
}  // namespace tracking::dqm

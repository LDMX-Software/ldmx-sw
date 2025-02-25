#pragma once

#include "Framework/Configure/Parameters.h"
#include "Framework/Event.h"
#include "Framework/EventProcessor.h"
#include "SimCore/Event/SimTrackerHit.h"
#include "Tracking/Event/Measurement.h"
#include "Tracking/Event/StraightTrack.h"

namespace tracking::dqm {

class StraightTracksDQM : public framework::Analyzer {
 public:
  StraightTracksDQM(const std::string& name, framework::Process& process)
      : framework::Analyzer(name, process){};

  /// Destructor
  ~StraightTracksDQM() = default;

  void analyze(const framework::Event& event) override;

  // Track monitoring for non-unique tracks (no truth comparison)
  void trackMonitoring(const std::vector<ldmx::StraightTrack>& tracks,
                       const std::vector<ldmx::Measurement>& measurements,
                       const std::string title, const bool& do_detail);

  // Track monitoring for unique tracks (has a truth comparison)
  void trackMonitoringUnique(const std::vector<ldmx::StraightTrack>& tracks,
                             const std::vector<ldmx::Measurement>& measurements,
                             const std::string title, const bool& do_detail,
                             const bool& do_truth);

  void configure(framework::config::Parameters& parameters) override;

  void onProcessEnd() override;

  // Distinguish which tracks are unique, duplicates, fake
  void sortTracks(const std::vector<ldmx::StraightTrack>& tracks,
                  std::vector<ldmx::StraightTrack>& unique_tracks,
                  std::vector<ldmx::StraightTrack>& duplicate_tracks,
                  std::vector<ldmx::StraightTrack>& fake_tracks);

  // Helper Functions: propogate track parameter error into calculation of
  // angles and ecal_locs I use error propagation based on partial derivatives
  // of functions that determine theta/phi/loc
  double thetaAngleError(double mx, double my,
                         const std::vector<double>& covariance_vector);
  double phiAngleError(double mx, const std::vector<double>& covariance_vector);
  double locError(double var_slope, double var_intercept,
                  double cov_slope_intercept, double z_pos);

 private:
  std::string track_collection_{"LinearRecoilTracks"};
  std::string truth_collection_{"LinearRecoilTruthTracks"};
  std::string measurement_collection_{"DigiRecoilSimHits"};
  std::string title_{"recoil_lin_trk_"};
  double track_prob_cut_{0.5};
  std::string subdetector_{"Recoil"};
  bool do_truth_comparison_{false};
  bool debug_{false};

  // Truth Track collection
  std::shared_ptr<ldmx::StraightTracks> truth_track_collection_{nullptr};

  // If I have truth information, sort the tracks vector according to their
  // trackID and truthProb
  // real tracks (truth_prob > cut), unique
  std::vector<ldmx::StraightTrack> unique_tracks_;
  // real tracks (truth_prob > cut), duplicated
  std::vector<ldmx::StraightTrack> duplicate_tracks_;
  // fake tracks (truth_prob < cut)
  std::vector<ldmx::StraightTrack> fake_tracks_;
};
}  // namespace tracking::dqm

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
  void TrackMonitoring(const std::vector<ldmx::StraightTrack>& tracks,
                       const std::vector<ldmx::Measurement>& measurements,
                       const std::string title, const bool& doDetail);

  // Track monitoring for unique tracks (has a truth comparison)
  void TrackMonitoringUnique(const std::vector<ldmx::StraightTrack>& tracks,
                             const std::vector<ldmx::Measurement>& measurements,
                             const std::string title, const bool& doDetail,
                             const bool& doTruth);

  void configure(framework::config::Parameters& parameters) override;

  void onProcessEnd() override;

  // Distinguish which tracks are unique, duplicates, fake
  void sortTracks(const std::vector<ldmx::StraightTrack>& tracks,
                  std::vector<ldmx::StraightTrack>& uniqueTracks,
                  std::vector<ldmx::StraightTrack>& duplicateTracks,
                  std::vector<ldmx::StraightTrack>& fakeTracks);

  // Helper Functions: propogate track parameter error into calculation of
  // angles and ecal_locs I use error propagation based on partial derivatives
  // of functions that determine theta/phi/loc
  double thetaAngleError(double mx, double my,
                         const std::vector<double>& covariance_vector);
  double phiAngleError(double mx, const std::vector<double>& covariance_vector);
  double locError(double var_slope, double var_intercept,
                  double cov_slope_intercept, double z_pos);

 private:
  std::string trackCollection_{"LinearRecoilTracks"};
  std::string truthCollection_{"LinearRecoilTruthTracks"};
  std::string measurementCollection_{"DigiRecoilSimHits"};
  std::string title_{"recoil_lin_trk_"};
  double trackProb_cut_{0.5};
  std::string subdetector_{"Recoil"};
  bool doTruthComparison{false};
  bool debug_{false};

  // Truth Track collection
  std::shared_ptr<ldmx::StraightTracks> truthTrackCollection_{nullptr};

  // If I have truth information, sort the tracks vector according to their
  // trackID and truthProb
  // real tracks (truth_prob > cut), unique
  std::vector<ldmx::StraightTrack> uniqueTracks_;
  // real tracks (truth_prob > cut), duplicated
  std::vector<ldmx::StraightTrack> duplicateTracks_;
  // fake tracks (truth_prob < cut)
  std::vector<ldmx::StraightTrack> fakeTracks_;
};
}  // namespace tracking::dqm

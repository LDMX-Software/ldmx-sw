#include "Tracking/dqm/StraightTracksDQM.h"

#include <algorithm>
#include <iostream>

#include "Tracking/Sim/TrackingUtils.h"

namespace tracking::dqm {

void StraightTracksDQM::configure(framework::config::Parameters& parameters) {
  track_collection_ = parameters.getParameter<std::string>(
      "track_collection", "LinearRecoilTracks");
  truth_collection_ = parameters.getParameter<std::string>(
      "truth_collection", "LinearRecoilTruthTracks");
  title_ = parameters.getParameter<std::string>("title", "recoil_lin_trk_");
  track_prob_cut_ = parameters.getParameter<double>("trackProb_cut", 0.5);
  subdetector_ = parameters.getParameter<std::string>("subdetector", "Recoil");
  measurement_collection_ = parameters.getParameter<std::string>(
      "measurement_collection", "DigiRecoilSimHits");
      
  track_collection_events_passname_ = parameters.getParameter<std::string>(
      "track_collection_events_passname");      
  truth_collection_events_passname_ = parameters.getParameter<std::string>(
      "truth_collection_events_passname"); 
  input_pass_name_ =
      parameters.getParameter<std::string>("input_pass_name");

  ldmx_log(info) << "Track Collection " << track_collection_;
  ldmx_log(info) << "Truth Collection " << truth_collection_;
}  // configure

void StraightTracksDQM::analyze(const framework::Event& event) {
  ldmx_log(debug) << "DQM Reading in::" << track_collection_;

  if (!event.exists(track_collection_, track_collection_events_passname_)) {
    ldmx_log(error) << "trackCollection " << track_collection_
                    << " not in event";
    return;
  }

  const std::vector<ldmx::StraightTrack> tracks =
      event.getCollection<ldmx::StraightTrack>(track_collection_,
                                               input_pass_name_);
  const std::vector<ldmx::Measurement> measurements =
      event.getCollection<ldmx::Measurement>(measurement_collection_,
                                             input_pass_name_);

  // Get the truth track collection
  if (event.exists(truth_collection_, truth_collection_events_passname_)) {
    truth_track_collection_ =
        std::make_shared<std::vector<ldmx::StraightTrack>>(
            event.getCollection<ldmx::StraightTrack>(truth_collection_,
                                                     input_pass_name_));
    do_truth_comparison_ = true;
  }

  ldmx_log(debug) << "Do truth comparison::" << do_truth_comparison_;

  if (do_truth_comparison_) {
    sortTracks(tracks, unique_tracks_, duplicate_tracks_, fake_tracks_);
  } else {
    unique_tracks_ = tracks;
  }

  ldmx_log(debug) << "Filling histograms ";

  // General Plots
  histograms_.fill(title_ + "N_tracks", tracks.size());

  ldmx_log(debug) << "Track Monitoring on Unique Tracks";

  trackMonitoringUnique(unique_tracks_, measurements, title_, true, true);

  ldmx_log(debug) << "Track Monitoring on duplicates and fakes";

  // Fakes and duplicates
  trackMonitoring(duplicate_tracks_, measurements, title_ + "dup_", false);
  trackMonitoring(fake_tracks_, measurements, title_ + "fake_", false);

  // Clear the vectors
  unique_tracks_.clear();
  duplicate_tracks_.clear();
  fake_tracks_.clear();
}  // analyze

void StraightTracksDQM::trackMonitoring(
    const std::vector<ldmx::StraightTrack>& tracks,
    const std::vector<ldmx::Measurement>& measurements, const std::string title,
    const bool& do_detail) {
  for (auto& track : tracks) {
    double trk_theta = track.getTheta();
    double trk_phi = track.getPhi();
    double track_state_loc0_target = track.getTargetX();
    double track_state_loc1_target = track.getTargetY();
    double track_state_loc0_ecal = track.getEcalLayer1X();
    double track_state_loc1_ecal = track.getEcalLayer1Y();
    int track_pdgID = track.getPdgID();

    double sigma_phi = phiAngleError(track.getSlopeX(), track.getCov());
    double sigma_theta =
        thetaAngleError(track.getSlopeX(), track.getSlopeY(), track.getCov());

    histograms_.fill(title + "phi", trk_phi);
    histograms_.fill(title + "theta", trk_theta);
    histograms_.fill(title_ + "trk_target_loc0", track_state_loc0_target);
    histograms_.fill(title_ + "trk_target_loc1", track_state_loc1_target);
    histograms_.fill(title_ + "trk_ecal_loc0", track_state_loc0_ecal);
    histograms_.fill(title_ + "trk_ecal_loc1", track_state_loc1_ecal);
    histograms_.fill(title + "trk_PID", track_pdgID);

    if (do_detail) {
      histograms_.fill(title + "nHits", track.getNhits());
      histograms_.fill(title + "Chi2", track.getChi2());
      histograms_.fill(title + "ndf", track.getNdf());
      histograms_.fill(title + "Chi2_per_ndf",
                       track.getChi2() / track.getNdf());
      histograms_.fill(title + "dRecHit", track.getDistanceToRecHit());

      histograms_.fill(title + "phi_err", sigma_phi);
      histograms_.fill(title + "theta_err", sigma_theta);
    }  // do detail

  }  // for tracks
}  // TrackMonitoring

void StraightTracksDQM::trackMonitoringUnique(
    const std::vector<ldmx::StraightTrack>& tracks,
    const std::vector<ldmx::Measurement>& measurements, const std::string title,
    const bool& do_detail, const bool& do_truth) {
  for (auto& track : tracks) {
    double trk_theta = track.getTheta();
    double trk_phi = track.getPhi();
    double track_state_loc0_target = track.getTargetX();
    double track_state_loc1_target = track.getTargetY();
    double track_state_loc0_ecal = track.getEcalLayer1X();
    double track_state_loc1_ecal = track.getEcalLayer1Y();
    int track_pdgID = track.getPdgID();

    double sigma_phi = phiAngleError(track.getSlopeX(), track.getCov());
    double sigma_theta =
        thetaAngleError(track.getSlopeX(), track.getSlopeY(), track.getCov());
    double sigma_loc0_target = std::sqrt(track.getCov()[4]);
    double sigma_loc1_target = std::sqrt(track.getCov()[9]);
    double sigma_loc0_ecal =
        locError(track.getCov()[0], track.getCov()[4], track.getCov()[1],
                 track.getEcalLayer1Z());
    double sigma_loc1_ecal =
        locError(track.getCov()[7], track.getCov()[9], track.getCov()[8],
                 track.getEcalLayer1Z());

    histograms_.fill(title + "phi", trk_phi);
    histograms_.fill(title + "theta", trk_theta);
    histograms_.fill(title_ + "trk_target_loc0", track_state_loc0_target);
    histograms_.fill(title_ + "trk_target_loc1", track_state_loc1_target);
    histograms_.fill(title_ + "trk_ecal_loc0", track_state_loc0_ecal);
    histograms_.fill(title_ + "trk_ecal_loc1", track_state_loc1_ecal);
    histograms_.fill(title + "trk_PID", track_pdgID);

    if (do_detail) {
      histograms_.fill(title + "nHits", track.getNhits());
      histograms_.fill(title + "Chi2", track.getChi2());
      histograms_.fill(title + "ndf", track.getNdf());
      histograms_.fill(title + "Chi2_per_ndf",
                       track.getChi2() / track.getNdf());
      histograms_.fill(title + "dRecHit", track.getDistanceToRecHit());

      histograms_.fill(title + "phi_err", sigma_phi);
      histograms_.fill(title + "theta_err", sigma_theta);

      if (do_truth) {
        // Match to the truth track
        // TODO: Currently, we assume the truth tracks have no uncertainty, but
        // the parameters for the truth tracks
        // TODO: are found from a fitting algorithm, and so have some
        // uncertainty. Is there a better way to represent
        // TODO: the truth tracks for a more accurate comparison?
        ldmx::StraightTrack* truth_trk = nullptr;

        // Only compare truth and reco tracks with same ID
        auto it = std::find_if(truth_track_collection_->begin(),
                               truth_track_collection_->end(),
                               [&](const ldmx::StraightTrack& tt) {
                                 return tt.getTrackID() == track.getTrackID();
                               });

        double track_truth_prob = track.getTruthProb();

        // make sure truth track has good enough truth prob.
        if (it != truth_track_collection_->end() &&
            track_truth_prob >= track_prob_cut_) {
          truth_trk = &(*it);
        }

        // Found matched track
        if (truth_trk) {
          double truth_theta = truth_trk->getTheta();
          double truth_phi = truth_trk->getPhi();
          double truth_state_loc0_target = truth_trk->getTargetX();
          double truth_state_loc1_target = truth_trk->getTargetY();
          double truth_state_loc0_ecal = truth_trk->getEcalLayer1X();
          double truth_state_loc1_ecal = truth_trk->getEcalLayer1Y();
          int truth_pdgID = truth_trk->getPdgID();

          histograms_.fill(title + "truth_phi", truth_phi);
          histograms_.fill(title + "truth_theta", truth_theta);
          histograms_.fill(title + "truth_PID", truth_pdgID);

          double res_phi = trk_phi - truth_phi;
          double res_theta = trk_theta - truth_theta;

          histograms_.fill(title + "res_phi", res_phi);
          histograms_.fill(title + "res_theta", res_theta);

          double pull_phi = res_phi / sigma_phi;
          double pull_theta = res_theta / sigma_theta;
          histograms_.fill(title + "pull_phi", pull_phi);
          histograms_.fill(title + "pull_theta", pull_theta);

          // TH1F  The difference(residual) between end_loc and truth_endloc
          histograms_.fill(title_ + "trk_target_loc0-truth_target_loc0",
                           track_state_loc0_target - truth_state_loc0_target);
          histograms_.fill(title_ + "trk_target_loc1-truth_target_loc1",
                           track_state_loc1_target - truth_state_loc1_target);
          histograms_.fill(title_ + "trk_ecal_loc0-truth_ecal_loc0",
                           track_state_loc0_ecal - truth_state_loc0_ecal);
          histograms_.fill(title_ + "trk_ecal_loc1-truth_ecal_loc1",
                           track_state_loc1_ecal - truth_state_loc1_ecal);

          // TH1F  The pulls of loc0 and loc1
          histograms_.fill(title_ + "target_Pulls_of_loc0",
                           (track_state_loc0_target - truth_state_loc0_target) /
                               sigma_loc0_target);
          histograms_.fill(title_ + "target_Pulls_of_loc1",
                           (track_state_loc1_target - truth_state_loc1_target) /
                               sigma_loc1_target);
          histograms_.fill(title_ + "ecal_Pulls_of_loc0",
                           (track_state_loc0_ecal - truth_state_loc0_ecal) /
                               sigma_loc0_ecal);
          histograms_.fill(title_ + "ecal_Pulls_of_loc1",
                           (track_state_loc1_ecal - truth_state_loc1_ecal) /
                               sigma_loc1_ecal);

          // TH2F  residual vs Nhits
          histograms_.fill(title_ + "target_res_loc0-vs-N_hits",
                           track.getNhits(),
                           track_state_loc0_target - truth_state_loc0_target);
          histograms_.fill(title_ + "target_res_loc1-vs-N_hits",
                           track.getNhits(),
                           track_state_loc1_target - truth_state_loc1_target);
          histograms_.fill(title_ + "ecal_res_loc0-vs-N_hits", track.getNhits(),
                           track_state_loc0_ecal - truth_state_loc0_ecal);
          histograms_.fill(title_ + "ecal_res_loc1-vs-N_hits", track.getNhits(),
                           track_state_loc1_ecal - truth_state_loc1_ecal);

          // TH2F  pulls vs Nhits
          histograms_.fill(title_ + "target_pulls_loc0-vs-N_hits",
                           track.getNhits(),
                           (track_state_loc0_target - truth_state_loc0_target) /
                               sigma_loc0_target);
          histograms_.fill(title_ + "target_pulls_loc1-vs-N_hits",
                           track.getNhits(),
                           (track_state_loc1_target - truth_state_loc1_target) /
                               sigma_loc1_target);
          histograms_.fill(title_ + "ecal_pulls_loc0-vs-N_hits",
                           track.getNhits(),
                           (track_state_loc0_ecal - truth_state_loc0_ecal) /
                               sigma_loc0_ecal);
          histograms_.fill(title_ + "ecal_pulls_loc1-vs-N_hits",
                           track.getNhits(),
                           (track_state_loc1_ecal - truth_state_loc1_ecal) /
                               sigma_loc1_ecal);

        }  // loop on tracks

      }  // do truth
    }  // do detail

  }  // for tracks
}  // TrackMonitoringUnique

void StraightTracksDQM::sortTracks(
    const std::vector<ldmx::StraightTrack>& tracks,
    std::vector<ldmx::StraightTrack>& unique_tracks,
    std::vector<ldmx::StraightTrack>& duplicate_tracks,
    std::vector<ldmx::StraightTrack>& fake_tracks) {
  std::vector<ldmx::StraightTrack> sorted_tracks = tracks;

  // Sort the vector of Track objects based on their trackID member
  std::sort(sorted_tracks.begin(), sorted_tracks.end(),
            [](ldmx::StraightTrack& trk1, ldmx::StraightTrack& trk2) {
              return trk1.getTrackID() < trk2.getTrackID();
            });

  // Loop over the sorted vector of Track objects
  for (size_t i = 0; i < sorted_tracks.size(); i++) {
    if (sorted_tracks[i].getTruthProb() < track_prob_cut_)
      fake_tracks.push_back(sorted_tracks[i]);
    else {
      // If this is the first Track object with this trackID, add it to the
      // uniqueTracks vector directly
      if (unique_tracks.size() == 0 ||
          sorted_tracks[i].getTrackID() != sorted_tracks[i - 1].getTrackID()) {
        unique_tracks.push_back(sorted_tracks[i]);
      }

      // Otherwise, add it to the duplicateTracks vector if its truthProb is
      // lower than the existing Track object Otherwise, if the truthProbability
      // is higher than the track stored in uniqueTracks, put it in uniqueTracks
      // and move the uniqueTracks.back to duplicateTracks.
      else if (sorted_tracks[i].getTruthProb() >
               unique_tracks.back().getTruthProb()) {
        duplicate_tracks.push_back(unique_tracks.back());
        unique_tracks.back() = sorted_tracks[i];
      }

      // Otherwise, add it to the duplicateTracks vector
      else {
        duplicate_tracks.push_back(sorted_tracks[i]);
      }
    }  // else (a real track)
  }  // loop on sorted tracks

  // The total number of elements in the uniqueTracks and duplicateTracks
  // vectors should be equal to the number of elements in the original tracks
  // vector
  if ((unique_tracks.size() + duplicate_tracks.size() + fake_tracks.size()) !=
      tracks.size()) {
    ldmx_log(error) << "Unique and duplicate track vectors do not add up to "
                       "original tracks vector";
    return;
  }  // if different tracks don't add up to correct total

  // Iterate through the uniqueTracks vector and duplicateTracks vector
  ldmx_log(trace) << "Unique tracks:";
  for (const ldmx::StraightTrack& track : unique_tracks) {
    ldmx_log(trace) << "Track ID: " << track.getTrackID()
                    << ", Truth Prob: " << track.getTruthProb();
  }
  ldmx_log(trace) << "Duplicate tracks:";
  for (const ldmx::StraightTrack& track : duplicate_tracks) {
    ldmx_log(trace) << "Track ID: " << track.getTrackID()
                    << ", Truth Prob: " << track.getTruthProb();
  }
  ldmx_log(trace) << "Fake tracks:";
  for (const ldmx::StraightTrack& track : fake_tracks) {
    ldmx_log(trace) << "Track ID: " << track.getTrackID()
                    << ", Truth Prob: " << track.getTruthProb();
  }

}  // sortTracks

double StraightTracksDQM::thetaAngleError(
    double m_x, double m_y, const std::vector<double>& covariance_vector) {
  double sqrt_term = std::sqrt(1 + (m_x * m_x));
  double sum_term = (1 + (m_x * m_x) + (m_y * m_y));

  double dtheta_dmx = (-m_x * m_y) / (sqrt_term * sum_term);
  double dtheta_dmy = (sqrt_term / sum_term);

  double sigma_mx2 = covariance_vector[0];
  double sigma_my2 = covariance_vector[7];
  double cov_mx_my = covariance_vector[2];

  double sigma_theta2 = (dtheta_dmx * dtheta_dmx * sigma_mx2) +
                        (dtheta_dmy * dtheta_dmy * sigma_my2) +
                        (2 * dtheta_dmx * dtheta_dmy * cov_mx_my);

  return std::sqrt(sigma_theta2);
}  // thetaAngleError

double StraightTracksDQM::phiAngleError(
    double m_x, const std::vector<double>& covariance_vector) {
  double sum_term = (1 + (m_x * m_x));

  double sigma_mx = std::sqrt(covariance_vector[0]);

  double sigma_phi = (sigma_mx / sum_term);

  return sigma_phi;
}  // phiAngleError

double StraightTracksDQM::locError(double var_slope, double var_intercept,
                                   double cov_slope_intercept, double z_pos) {
  return std::sqrt((z_pos * z_pos * var_slope) + var_intercept +
                   (2 * z_pos * cov_slope_intercept));
}  // locAngleError

}  // namespace tracking::dqm

DECLARE_ANALYZER(tracking::dqm::StraightTracksDQM)

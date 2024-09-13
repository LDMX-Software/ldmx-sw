#include "Tracking/dqm/TrackingRecoDQM.h"

#include <algorithm>
#include <iostream>

#include "Tracking/Sim/TrackingUtils.h"

namespace tracking::dqm {

void TrackingRecoDQM::configure(framework::config::Parameters& parameters) {
  trackCollection_ =
      parameters.getParameter<std::string>("track_collection", "TaggerTracks");
  truthCollection_ = parameters.getParameter<std::string>("truth_collection",
                                                          "TaggerTruthTracks");
  title_ = parameters.getParameter<std::string>("title", "tagger_trk_");
  trackProb_cut_ = parameters.getParameter<double>("trackProb_cut", 0.5);
  subdetector_ = parameters.getParameter<std::string>("subdetector", "Tagger");
  trackStates_ =
      parameters.getParameter<std::vector<std::string>>("trackStates", {});
  measurementCollection_ = parameters.getParameter<std::string>(
      "measurement_collection", "DigiTaggerSimHits");

  ldmx_log(info) << "Track Collection " << trackCollection_ << std::endl;
  ldmx_log(info) << "Truth Collection " << truthCollection_ << std::endl;

  pidmap[-321] = PIDBins::kminus;
  pidmap[321] = PIDBins::kplus;
  pidmap[-211] = PIDBins::piminus;
  pidmap[211] = PIDBins::piplus;
  pidmap[11] = PIDBins::electron;
  pidmap[-11] = PIDBins::positron;
  pidmap[2212] = PIDBins::proton;
  pidmap[-2212] = PIDBins::antiproton;
}

void TrackingRecoDQM::analyze(const framework::Event& event) {
  ldmx_log(debug) << "DQM Reading in::" << trackCollection_ << std::endl;

  if (!event.exists(trackCollection_)) {
    ldmx_log(error) << "ERROR:: trackCollection " << trackCollection_
                    << " not in event" << std::endl;
    return;
  }
  auto tracks{event.getCollection<ldmx::Track>(trackCollection_)};
  auto measurements{
      event.getCollection<ldmx::Measurement>(measurementCollection_)};
  // The truth track collection
  if (event.exists(truthCollection_)) {
    truthTrackCollection_ = std::make_shared<ldmx::Tracks>(
        event.getCollection<ldmx::Track>(truthCollection_));
    doTruthComparison = true;
  }

  // The scoring plane hits
  if (event.exists("EcalScoringPlaneHits")) {
    ecal_scoring_hits_ = std::make_shared<std::vector<ldmx::SimTrackerHit>>(
        event.getCollection<ldmx::SimTrackerHit>("EcalScoringPlaneHits"));
  }

  if (event.exists("TargetScoringPlaneHits")) {
    target_scoring_hits_ = std::make_shared<std::vector<ldmx::SimTrackerHit>>(
        event.getCollection<ldmx::SimTrackerHit>("TargetScoringPlaneHits"));
  }

  ldmx_log(debug) << "Do truth comparison::" << doTruthComparison << std::endl;

  if (doTruthComparison) {
    sortTracks(tracks, uniqueTracks, duplicateTracks, fakeTracks);
  } else {
    uniqueTracks = tracks;
  }

  ldmx_log(debug) << "Filling histograms " << std::endl;

  // General Plots
  histograms_.fill(title_ + "N_tracks", tracks.size());

  ldmx_log(debug) << "Track Monitoring on Unique Tracks" << std::endl;
  TrackMonitoring(uniqueTracks, measurements, title_, true, true);

  ldmx_log(debug) << "Track Monitoring on duplicates and fakes" << std::endl;
  // Fakes and duplicates
  TrackMonitoring(duplicateTracks, measurements, title_ + "dup_", false, false);
  TrackMonitoring(fakeTracks, measurements, title_ + "fake_", false, false);

  // Track Extrapolation to Ecal Monitoring

  if (std::find(trackStates_.begin(), trackStates_.end(), "target") !=
      trackStates_.end()) {
    TrackStateMonitoring(tracks, ldmx::TrackStateType::AtTarget, "target");
  }

  if (std::find(trackStates_.begin(), trackStates_.end(), "ecal") !=
      trackStates_.end()) {
    TrackStateMonitoring(tracks, ldmx::TrackStateType::AtECAL, "ecal");
  }

  if (std::find(trackStates_.begin(), trackStates_.end(), "beamOrigin") !=
      trackStates_.end()) {
    TrackStateMonitoring(tracks, ldmx::TrackStateType::AtBeamOrigin,
                         "beamOrigin");
  }

  // Technical Efficiency plots
  EfficiencyPlots(tracks, measurements, title_);

  // Tagger Recoil Matching

  // Clear the vectors
  uniqueTracks.clear();
  duplicateTracks.clear();
  fakeTracks.clear();
}

void TrackingRecoDQM::onProcessEnd() {
  // Produce the efficiency plots. (TODO::Switch to TEfficiency instead)
}

void TrackingRecoDQM::EfficiencyPlots(
    const std::vector<ldmx::Track>& tracks,
    const std::vector<ldmx::Measurement>& measurements,
    const std::string& title) {
  // Do all truth track plots - denominator

  histograms_.fill(title + "truth_N_tracks", truthTrackCollection_->size());
  for (auto& truth_trk : *(truthTrackCollection_)) {
    double truth_phi = truth_trk.getPhi();
    double truth_d0 = truth_trk.getD0();
    double truth_z0 = truth_trk.getZ0();
    double truth_theta = truth_trk.getTheta();
    double truth_qop = truth_trk.getQoP();
    double truth_p = 1. / abs(truth_trk.getQoP());
    int truth_nHits = truth_trk.getNhits();

    std::vector<double> truth_mom = truth_trk.getMomentum();

    // Polar angle
    // The momentum in the plane transverse wrt the beam axis
    double truth_pt_beam =
        std::sqrt(truth_mom[1] * truth_mom[1] + truth_mom[2] * truth_mom[2]);

    double truth_beam_angle = std::atan2(truth_pt_beam, truth_mom[0]);

    histograms_.fill(title + "truth_nHits", truth_nHits);
    histograms_.fill(title + "truth_d0", truth_d0);
    histograms_.fill(title + "truth_z0", truth_z0);
    histograms_.fill(title + "truth_phi", truth_phi);
    histograms_.fill(title + "truth_theta", truth_theta);
    histograms_.fill(title + "truth_qop", truth_qop);
    histograms_.fill(title + "truth_p", truth_p);
    histograms_.fill(title + "truth_beam_angle", truth_beam_angle);

    if (pidmap.count(truth_trk.getPdgID()) != 0) {
      histograms_.fill(title + "truth_PID", pidmap[truth_trk.getPdgID()]);

      // TODO do this properly.

      if (pidmap[truth_trk.getPdgID()] == PIDBins::kminus) {
        histograms_.fill(title + "truth_kminus_p", truth_p);
      }

      if (pidmap[truth_trk.getPdgID()] == PIDBins::kplus) {
        histograms_.fill(title + "truth_kplus_p", truth_p);
      }

      if (pidmap[truth_trk.getPdgID()] == PIDBins::piminus) {
        histograms_.fill(title + "truth_piminus_p", truth_p);
      }

      if (pidmap[truth_trk.getPdgID()] == PIDBins::piplus) {
        histograms_.fill(title + "truth_piplus_p", truth_p);
      }

      if (pidmap[truth_trk.getPdgID()] == PIDBins::electron) {
        histograms_.fill(title + "truth_electron_p", truth_p);
      }

      if (pidmap[truth_trk.getPdgID()] == PIDBins::positron) {
        histograms_.fill(title + "truth_positron_p", truth_p);
      }

      if (pidmap[truth_trk.getPdgID()] == PIDBins::proton) {
        histograms_.fill(title + "truth_proton_p", truth_p);
      }
    }

  }  // loop on truth tracks

  for (auto& track : tracks) {
    // Match the tracks to truth
    ldmx::Track* truth_trk = nullptr;

    auto it =
        std::find_if(truthTrackCollection_->begin(),
                     truthTrackCollection_->end(), [&](const ldmx::Track& tt) {
                       return tt.getTrackID() == track.getTrackID();
                     });

    double trackTruthProb = track.getTruthProb();

    if (it != truthTrackCollection_->end() && trackTruthProb >= trackProb_cut_)
      truth_trk = &(*it);

    // Match not found
    if (!truth_trk) return;

    double truth_phi = truth_trk->getPhi();
    double truth_d0 = truth_trk->getD0();
    double truth_z0 = truth_trk->getZ0();
    double truth_theta = truth_trk->getTheta();
    double truth_qop = truth_trk->getQoP();
    double truth_p = 1. / abs(truth_trk->getQoP());
    std::vector<double> truth_mom = truth_trk->getMomentum();

    // Polar angle
    // The momentum in the plane transverse wrt the beam axis
    double truth_pt_beam =
        std::sqrt(truth_mom[1] * truth_mom[1] + truth_mom[2] * truth_mom[2]);

    double truth_beam_angle = std::atan2(truth_pt_beam, truth_mom[0]);

    // Fill reco plots for efficiencies - numerator. The quantities are truth
    histograms_.fill(title + "match_prob", trackTruthProb);
    histograms_.fill(title + "match_d0", truth_d0);
    histograms_.fill(title + "match_z0", truth_z0);
    histograms_.fill(title + "match_phi", truth_phi);
    histograms_.fill(title + "match_theta", truth_theta);
    histograms_.fill(title + "match_p", truth_p);
    histograms_.fill(title + "match_qop", truth_qop);
    histograms_.fill(title + "match_beam_angle", truth_beam_angle);
    histograms_.fill(title + "match_nHits", measurements.size());
    for (auto trackHit : track.getMeasurementsIdxs()) {
      histograms_.fill(title + "match_LayersHit",
                       measurements.at(trackHit).getLayer());
    }

    // For some particles

    if (pidmap.count(truth_trk->getPdgID()) != 0) {
      histograms_.fill(title + "match_PID", pidmap[truth_trk->getPdgID()]);

      // TODO do this properly.

      if (pidmap[truth_trk->getPdgID()] == PIDBins::kminus) {
        histograms_.fill(title + "match_kminus_p", truth_p);
      }

      if (pidmap[truth_trk->getPdgID()] == PIDBins::kplus) {
        histograms_.fill(title + "match_kplus_p", truth_p);
      }

      if (pidmap[truth_trk->getPdgID()] == PIDBins::piminus) {
        histograms_.fill(title + "match_piminus_p", truth_p);
      }

      if (pidmap[truth_trk->getPdgID()] == PIDBins::piplus) {
        histograms_.fill(title + "match_piplus_p", truth_p);
      }

      if (pidmap[truth_trk->getPdgID()] == PIDBins::electron) {
        histograms_.fill(title + "match_electron_p", truth_p);
      }

      if (pidmap[truth_trk->getPdgID()] == PIDBins::positron) {
        histograms_.fill(title + "match_positron_p", truth_p);
      }

      if (pidmap[truth_trk->getPdgID()] == PIDBins::proton) {
        histograms_.fill(title + "match_proton_p", truth_p);
      }
    }
  }  // Loop on tracks

}  // Efficiency plots

void TrackingRecoDQM::TrackMonitoring(
    const std::vector<ldmx::Track>& tracks,
    const std::vector<ldmx::Measurement>& measurements, const std::string title,
    const bool& doDetail, const bool& doTruth) {
  for (auto& track : tracks) {
    // Perigee track parameters
    double trk_d0 = track.getD0();
    double trk_z0 = track.getZ0();
    double trk_qop = track.getQoP();
    double trk_theta = track.getTheta();
    double trk_phi = track.getPhi();
    double trk_p = 1. / abs(trk_qop);
    for (auto trackHit : track.getMeasurementsIdxs()) {
      histograms_.fill(title + "LayersHit",
                       measurements.at(trackHit).getLayer());
    }

    std::vector<double> trk_mom = track.getMomentum();

    // The transverse momentum in the bending plane
    double pt_bending =
        std::sqrt(trk_mom[0] * trk_mom[0] + trk_mom[1] * trk_mom[1]);

    // The momentum in the plane transverse wrt the beam axis
    double pt_beam =
        std::sqrt(trk_mom[1] * trk_mom[1] + trk_mom[2] * trk_mom[2]);

    // Covariance matrix
    Acts::BoundSquareMatrix cov =
        tracking::sim::utils::unpackCov(track.getPerigeeCov());

    double sigmad0 = sqrt(
        cov(Acts::BoundIndices::eBoundLoc0, Acts::BoundIndices::eBoundLoc0));
    double sigmaz0 = sqrt(
        cov(Acts::BoundIndices::eBoundLoc1, Acts::BoundIndices::eBoundLoc1));
    double sigmaphi =
        sqrt(cov(Acts::BoundIndices::eBoundPhi, Acts::BoundIndices::eBoundPhi));
    double sigmatheta = sqrt(
        cov(Acts::BoundIndices::eBoundTheta, Acts::BoundIndices::eBoundTheta));
    double sigmaqop = sqrt(cov(Acts::BoundIndices::eBoundQOverP,
                               Acts::BoundIndices::eBoundQOverP));
    double sigmap = (1. / trk_qop) * (1. / trk_qop) * sigmaqop;

    histograms_.fill(title + "d0", trk_d0);
    histograms_.fill(title + "z0", trk_z0);
    histograms_.fill(title + "qop", trk_qop);
    histograms_.fill(title + "phi", trk_phi);
    histograms_.fill(title + "theta", trk_theta);
    histograms_.fill(title + "p", std::abs(1. / trk_qop));

    if (doDetail) {
      histograms_.fill(title + "px", trk_mom[0]);
      histograms_.fill(title + "py", trk_mom[1]);
      histograms_.fill(title + "pz", trk_mom[2]);

      histograms_.fill(title + "pt_bending", pt_bending);
      histograms_.fill(title + "pt_beam", pt_beam);

      histograms_.fill(title + "nHits", track.getNhits());
      histograms_.fill(title + "Chi2", track.getChi2());
      histograms_.fill(title + "ndf", track.getNdf());
      histograms_.fill(title + "Chi2/ndf", track.getChi2() / track.getNdf());
      histograms_.fill(title + "nShared", track.getNsharedHits());

      histograms_.fill(title + "d0_err", sigmad0);
      histograms_.fill(title + "z0_err", sigmaz0);
      histograms_.fill(title + "phi_err", sigmaphi);
      histograms_.fill(title + "theta_err", sigmatheta);
      histograms_.fill(title + "qop_err", sigmaqop);
      histograms_.fill(title + "p_err", sigmap);

      // 2D Error plots

      double p = std::abs(1. / trk_qop);
      histograms_.fill(title + "d0_err_vs_p", p, sigmad0);
      histograms_.fill(title + "z0_err_vs_p", std::abs(1. / trk_qop), sigmaz0);
      histograms_.fill(title + "p_err_vs_p", std::abs(1. / trk_qop), sigmap);

      if (track.getNhits() == 8)
        histograms_.fill(title + "p_err_vs_p_8hits", p, sigmap);
      else if (track.getNhits() == 9)
        histograms_.fill(title + "p_err_vs_p_9hits", p, sigmap);
      else if (track.getNhits() == 10)
        histograms_.fill(title + "p_err_vs_p_10hits", p, sigmap);
    }

    if (doTruth) {
      // Match to the truth track
      ldmx::Track* truth_trk = nullptr;

      auto it = std::find_if(truthTrackCollection_->begin(),
                             truthTrackCollection_->end(),
                             [&](const ldmx::Track& tt) {
                               return tt.getTrackID() == track.getTrackID();
                             });

      double trackTruthProb = track.getTruthProb();

      if (it != truthTrackCollection_->end() &&
          trackTruthProb >= trackProb_cut_)
        truth_trk = &(*it);

      // Found matched track
      if (truth_trk) {
        double truth_d0 = truth_trk->getD0();
        double truth_z0 = truth_trk->getZ0();
        double truth_phi = truth_trk->getPhi();
        double truth_theta = truth_trk->getTheta();
        double truth_qop = truth_trk->getQoP();
        double truth_p = 1. / abs(truth_trk->getQoP());
        std::vector<double> truth_mom = truth_trk->getMomentum();
        // Polar angle
        // The momentum in the plane transverse wrt the beam axis
        double truth_pt_beam = std::sqrt(truth_mom[1] * truth_mom[1] +
                                         truth_mom[2] * truth_mom[2]);

        // histograms_.fill(title+"truth_d0",   truth_d0);
        // histograms_.fill(title+"truth_z0",   truth_z0);
        // histograms_.fill(title+"truth_phi",  truth_phi);
        // histograms_.fill(title+"truth_theta",truth_theta);
        // histograms_.fill(title+"truth_qop",  truth_qop);
        // histograms_.fill(title+"truth_p",    truth_p);

        double res_d0 = trk_d0 - truth_d0;
        double res_z0 = trk_z0 - truth_z0;
        double res_phi = trk_phi - truth_phi;
        double res_theta = trk_theta - truth_theta;
        double res_qop = trk_qop - truth_qop;
        double res_p = trk_p - truth_p;
        double res_pt_beam = pt_beam - truth_pt_beam;

        histograms_.fill(title + "res_d0", res_d0);
        histograms_.fill(title + "res_z0", res_z0);
        histograms_.fill(title + "res_phi", res_phi);
        histograms_.fill(title + "res_theta", res_theta);
        histograms_.fill(title + "res_qop", res_qop);
        histograms_.fill(title + "res_p", res_p);
        histograms_.fill(title + "res_pt_beam", res_pt_beam);

        double pull_d0 = res_d0 / sigmad0;
        double pull_z0 = res_z0 / sigmaz0;
        double pull_phi = res_phi / sigmaphi;
        double pull_theta = res_theta / sigmatheta;
        double pull_qop = res_qop / sigmaqop;
        double pull_p = res_p / sigmap;

        histograms_.fill(title + "pull_d0", pull_d0);
        histograms_.fill(title + "pull_z0", pull_z0);
        histograms_.fill(title + "pull_phi", pull_phi);
        histograms_.fill(title + "pull_theta", pull_theta);
        histograms_.fill(title + "pull_qop", pull_qop);
        histograms_.fill(title + "pull_p", pull_p);

        // Error plots from residuals

        histograms_.fill(title + "res_p_vs_p", truth_p, res_p);

        histograms_.fill(title + "res_qop_vs_p", truth_p, res_qop);
        histograms_.fill(title + "res_d0_vs_p", truth_p, res_d0);
        histograms_.fill(title + "res_z0_vs_p", truth_p, res_z0);
        histograms_.fill(title + "res_phi_vs_p", truth_p, res_phi);
        histograms_.fill(title + "res_theta_vs_p", truth_p, res_theta);

        histograms_.fill(title + "pull_qop_vs_p", truth_p, pull_qop);
        histograms_.fill(title + "pull_d0_vs_p", truth_p, pull_d0);
        histograms_.fill(title + "pull_z0_vs_p", truth_p, pull_z0);
        histograms_.fill(title + "pull_phi_vs_p", truth_p, pull_phi);
        histograms_.fill(title + "pull_theta_vs_p", truth_p, pull_theta);

        if (track.getNhits() == 8)
          histograms_.fill(title + "res_p_vs_p_8hits", truth_p, res_p);
        else if (track.getNhits() == 9)
          histograms_.fill(title + "res_p_vs_p_9hits", truth_p, res_p);
        else if (track.getNhits() == 10)
          histograms_.fill(title + "res_p_vs_p_10hits", truth_p, res_p);

        histograms_.fill(title + "res_pt_beam_vs_p", truth_pt_beam,
                         res_pt_beam);

      }  // found matched track
    }    // do TruthComparison
  }      // loop on tracks

}  // Track Monitoring

void TrackingRecoDQM::TrackStateMonitoring(const ldmx::Tracks& tracks,
                                           ldmx::TrackStateType ts_type,
                                           const std::string& ts_title) {
  for (auto& track : tracks) {
    // Match the tracks to truth
    ldmx::Track* truth_trk = nullptr;

    auto it =
        std::find_if(truthTrackCollection_->begin(),
                     truthTrackCollection_->end(), [&](const ldmx::Track& tt) {
                       return tt.getTrackID() == track.getTrackID();
                     });

    double trackTruthProb = track.getTruthProb();

    if (it != truthTrackCollection_->end() && trackTruthProb >= trackProb_cut_)
      truth_trk = &(*it);

    // Match not found, skip track
    if (!truth_trk) continue;

    // TruthTrack doesn't have the right amount of states

    auto trk_ts = track.getTrackState(ts_type);
    auto truth_ts = truth_trk->getTrackState(ts_type);

    if (!trk_ts.has_value()) continue;

    if (!truth_ts.has_value()) continue;

    ldmx::Track::TrackState& truthTargetState = truth_ts.value();
    ldmx::Track::TrackState& TargetState = trk_ts.value();

    ldmx_log(debug) << "Unpacking covariance matrix" << std::endl;
    Acts::BoundSquareMatrix cov =
        tracking::sim::utils::unpackCov(TargetState.cov);

    double sigmaloc0 = sqrt(
        cov(Acts::BoundIndices::eBoundLoc0, Acts::BoundIndices::eBoundLoc0));
    double sigmaloc1 = sqrt(
        cov(Acts::BoundIndices::eBoundLoc1, Acts::BoundIndices::eBoundLoc1));
    double sigmaphi =
        sqrt(cov(Acts::BoundIndices::eBoundPhi, Acts::BoundIndices::eBoundPhi));
    double sigmatheta = sqrt(
        cov(Acts::BoundIndices::eBoundTheta, Acts::BoundIndices::eBoundTheta));
    double sigmaqop = sqrt(cov(Acts::BoundIndices::eBoundQOverP,
                               Acts::BoundIndices::eBoundQOverP));

    double trk_qop = track.getQoP();
    double trk_p = 1. / abs(trk_qop);

    double track_state_loc0 = TargetState.params[0];
    double track_state_loc1 = TargetState.params[1];
    double track_state_phi = TargetState.params[2];
    double track_state_theta = TargetState.params[3];
    double track_state_p = TargetState.params[4];

    double truth_state_loc0 = truthTargetState.params[0];
    double truth_state_loc1 = truthTargetState.params[1];
    double truth_state_phi = truthTargetState.params[2];
    double truth_state_theta = truthTargetState.params[3];
    double truth_state_p = truthTargetState.params[4];

    // Check that the track state is filled
    if (TargetState.params.size() < 5) continue;

    histograms_.fill(title_ + "trk_" + ts_title + "_loc0", track_state_loc0);
    histograms_.fill(title_ + "trk_" + ts_title + "_loc1", track_state_loc1);
    histograms_.fill(title_ + ts_title + "_sp_hit_X", truth_state_loc0);
    histograms_.fill(title_ + ts_title + "_sp_hit_Y", truth_state_loc1);

    // TH1F  The difference(residual) between end_loc0 and sp_hit_X
    histograms_.fill(title_ + "trk_" + ts_title + "_loc0-sp_hit_X",
                     track_state_loc0 - truth_state_loc0);
    histograms_.fill(title_ + "trk_" + ts_title + "_loc1-sp_hit_Y",
                     track_state_loc1 - truth_state_loc1);

    // TH1F  The pulls of loc0 and loc1
    histograms_.fill(title_ + ts_title + "_Pulls_of_loc0",
                     (track_state_loc0 - truth_state_loc0) / sigmaloc0);
    histograms_.fill(title_ + ts_title + "_Pulls_of_loc1",
                     (track_state_loc1 - truth_state_loc1) / sigmaloc1);

    // TODO:: TH1F The pulls of phi, theta, qop

    // TH2F  residual vs Nhits
    histograms_.fill(title_ + ts_title + "_res_loc0-vs-N_hits",
                     track.getNhits(), track_state_loc0 - truth_state_loc0);
    histograms_.fill(title_ + ts_title + "_res_loc1-vs-N_hits",
                     track.getNhits(), track_state_loc1 - truth_state_loc1);

    // TH2F  pulls vs Nhits
    histograms_.fill(title_ + ts_title + "_pulls_loc0-vs-N_hits",
                     track.getNhits(),
                     (track_state_loc0 - truth_state_loc0) / sigmaloc0);
    histograms_.fill(title_ + ts_title + "_pulls_loc1-vs-N_hits",
                     track.getNhits(),
                     (track_state_loc1 - truth_state_loc1) / sigmaloc1);

    // TH2F  residual vs trk_p
    histograms_.fill(title_ + ts_title + "_res_loc0-vs-trk_p", trk_p,
                     track_state_loc0 - truth_state_loc0);
    histograms_.fill(title_ + ts_title + "_res_loc1-vs-trk_p", trk_p,
                     track_state_loc1 - truth_state_loc1);

    // TH2F  pulls vs trk_p
    histograms_.fill(title_ + ts_title + "_pulls_loc0-vs-trk_p", trk_p,
                     (track_state_loc0 - truth_state_loc0) / sigmaloc0);
    histograms_.fill(title_ + ts_title + "_pulls_loc1-vs-trk_p", trk_p,
                     (track_state_loc1 - truth_state_loc1) / sigmaloc1);

  }  // loop on tracks
}

void TrackingRecoDQM::sortTracks(const std::vector<ldmx::Track>& tracks,
                                 std::vector<ldmx::Track>& uniqueTracks,
                                 std::vector<ldmx::Track>& duplicateTracks,
                                 std::vector<ldmx::Track>& fakeTracks) {
  // Create a copy of the const vector so we can sort it
  std::vector<ldmx::Track> sortedTracks = tracks;

  // Sort the vector of Track objects based on their trackID member
  std::sort(sortedTracks.begin(), sortedTracks.end(),
            [](ldmx::Track& t1, ldmx::Track& t2) {
              return t1.getTrackID() < t2.getTrackID();
            });

  // Loop over the sorted vector of Track objects
  for (size_t i = 0; i < sortedTracks.size(); i++) {
    if (sortedTracks[i].getTruthProb() < trackProb_cut_)
      fakeTracks.push_back(sortedTracks[i]);
    else {  // not a fake track
      // If this is the first Track object with this trackID, add it to the
      // uniqueTracks vector directly
      if (uniqueTracks.size() == 0 ||
          sortedTracks[i].getTrackID() != sortedTracks[i - 1].getTrackID()) {
        uniqueTracks.push_back(sortedTracks[i]);
      }
      // Otherwise, add it to the duplicateTracks vector if its truthProb is
      // lower than the existing Track object Otherwise, if the truthProbability
      // is higher than the track stored in uniqueTracks, put it in uniqueTracks
      // and move the uniqueTracks.back to duplicateTracks.
      else if (sortedTracks[i].getTruthProb() >
               uniqueTracks.back().getTruthProb()) {
        duplicateTracks.push_back(uniqueTracks.back());
        uniqueTracks.back() = sortedTracks[i];
      }
      // Otherwise, add it to the duplicateTracks vector
      else {
        duplicateTracks.push_back(sortedTracks[i]);
      }
    }  // a real track
  }    // loop on sorted tracks
  // The total number of elements in the uniqueTracks and duplicateTracks
  // vectors should be equal to the number of elements in the original tracks
  // vector
  if (uniqueTracks.size() + duplicateTracks.size() + fakeTracks.size() !=
      tracks.size()) {
    std::cerr << "Error: unique and duplicate tracks vectors do not add up to "
                 "original tracks vector"
              << std::endl;
    return;
  }

  if (debug_) {
    // Iterate through the uniqueTracks vector and duplicateTracks vector
    std::cout << "Unique tracks:" << std::endl;
    for (const ldmx::Track& track : uniqueTracks) {
      std::cout << "Track ID: " << track.getTrackID()
                << ", Truth Prob: " << track.getTruthProb() << std::endl;
    }
    std::cout << "Duplicate tracks:" << std::endl;
    for (const ldmx::Track& track : duplicateTracks) {
      std::cout << "Track ID: " << track.getTrackID()
                << ", Truth Prob: " << track.getTruthProb() << std::endl;
    }
    std::cout << "Fake tracks:" << std::endl;
    for (const ldmx::Track& track : fakeTracks) {
      std::cout << "Track ID: " << track.getTrackID()
                << ", Truth Prob: " << track.getTruthProb() << std::endl;
    }
  }
}
}  // namespace tracking::dqm

DECLARE_ANALYZER_NS(tracking::dqm, TrackingRecoDQM)

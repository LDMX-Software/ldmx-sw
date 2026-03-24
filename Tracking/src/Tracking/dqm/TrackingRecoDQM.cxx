#include "Tracking/dqm/TrackingRecoDQM.h"

namespace tracking::dqm {

void TrackingRecoDQM::configure(framework::config::Parameters& parameters) {
  track_collection_ = parameters.get<std::string>("track_collection");
  truth_collection_ = parameters.get<std::string>("truth_collection");
  measurement_collection_ =
      parameters.get<std::string>("measurement_collection");
  measurement_passname_ = parameters.get<std::string>("measurement_passname");

  ecal_sp_events_passname_ =
      parameters.get<std::string>("ecal_sp_events_passname");
  ecal_sp_passname_ = parameters.get<std::string>("ecal_sp_passname");
  target_sp_events_passname_ =
      parameters.get<std::string>("target_sp_events_passname");
  target_sp_passname_ = parameters.get<std::string>("target_sp_passname");
  truth_passname_ = parameters.get<std::string>("truth_passname");
  truth_events_passname_ = parameters.get<std::string>("truth_events_passname");
  track_passname_ = parameters.get<std::string>("track_passname");
  track_collection_events_passname_ =
      parameters.get<std::string>("track_collection_events_passname");

  title_ = parameters.get<std::string>("title", "tagger_trk_");
  track_prob_cut_ = parameters.get<double>("trackProb_cut", 0.5);
  subdetector_ = parameters.get<std::string>("subdetector", "Tagger");
  track_states_ = parameters.get<std::vector<std::string>>("trackStates", {});

  pidmap_[-321] = PIDBins::kminus;
  pidmap_[321] = PIDBins::kplus;
  pidmap_[-211] = PIDBins::piminus;
  pidmap_[211] = PIDBins::piplus;
  pidmap_[11] = PIDBins::electron;
  pidmap_[-11] = PIDBins::positron;
  pidmap_[2212] = PIDBins::proton;
  pidmap_[-2212] = PIDBins::antiproton;
}

void TrackingRecoDQM::analyze(const framework::Event& event) {
  ldmx_log(trace) << "DQM Reading in:" << track_collection_;

  if (!event.exists(track_collection_, track_collection_events_passname_)) {
    ldmx_log(error) << "TrackCollection " << track_collection_
                    << " with pass = " << track_collection_events_passname_
                    << " not in event";
    return;
  }

  auto tracks{
      event.getCollection<ldmx::Track>(track_collection_, track_passname_)};

  if (!event.exists(measurement_collection_, measurement_passname_)) {
    ldmx_log(error) << "Measurement collection " << measurement_collection_
                    << " with pass = " << measurement_passname_
                    << " not in event";
    return;
  }

  auto measurements{event.getCollection<ldmx::Measurement>(
      measurement_collection_, measurement_passname_)};

  // The truth track collection
  if (event.exists(truth_collection_, truth_events_passname_)) {
    truth_track_collection_ = std::make_shared<std::vector<ldmx::Track>>(
        event.getCollection<ldmx::Track>(truth_collection_, truth_passname_));
    do_truth_comparison_ = true;
  }

  // The scoring plane hits_
  if (event.exists("EcalScoringPlaneHits", ecal_sp_events_passname_)) {
    ecal_scoring_hits_ = std::make_shared<std::vector<ldmx::SimTrackerHit>>(
        event.getCollection<ldmx::SimTrackerHit>("EcalScoringPlaneHits",
                                                 ecal_sp_passname_));
  }

  if (event.exists("target_scoring_plane_hits", target_sp_events_passname_)) {
    target_scoring_hits_ = std::make_shared<std::vector<ldmx::SimTrackerHit>>(
        event.getCollection<ldmx::SimTrackerHit>("target_scoring_plane_hits",
                                                 target_sp_passname_));
  }

  ldmx_log(debug) << "Do truth comparison::" << do_truth_comparison_;

  if (do_truth_comparison_) {
    sortTracks(tracks, unique_tracks_, duplicate_tracks_, fake_tracks_);
  } else {
    unique_tracks_ = tracks;
  }

  ldmx_log(debug) << "Filling histograms for " << tracks.size() << " tracks";

  // General Plots
  histograms_.fill(title_ + "N_tracks", tracks.size());

  if (!unique_tracks_.empty()) {
    ldmx_log(debug) << "Track Monitoring on " << unique_tracks_.size()
                    << " Unique Tracks";
    trackMonitoring(unique_tracks_, measurements, title_, true,
                    do_truth_comparison_);
  }

  // Fakes and duplicates
  if (!duplicate_tracks_.empty()) {
    ldmx_log(debug) << "Track Monitoring on " << duplicate_tracks_.size()
                    << " duplicates";
    trackMonitoring(duplicate_tracks_, measurements, title_ + "dup_", false,
                    false);
  }
  if (!fake_tracks_.empty()) {
    ldmx_log(debug) << "Track Monitoring on " << fake_tracks_.size()
                    << " fakes";
    trackMonitoring(fake_tracks_, measurements, title_ + "fake_", false, false);
  }

  // Track Extrapolation to Ecal Monitoring
  ldmx_log(trace) << "Track Extrapolation to Ecal Monitoring";
  if (std::find(track_states_.begin(), track_states_.end(), "target") !=
      track_states_.end()) {
    trackStateMonitoring(tracks, ldmx::AtTarget, "target");
  }

  if (std::find(track_states_.begin(), track_states_.end(), "ecal") !=
      track_states_.end()) {
    trackStateMonitoring(tracks, ldmx::AtECAL, "ecal");
  }

  if (std::find(track_states_.begin(), track_states_.end(), "beamOrigin") !=
      track_states_.end()) {
    trackStateMonitoring(tracks, ldmx::AtBeamOrigin, "beamOrigin");
  }

  // Technical Efficiency plots
  if (do_truth_comparison_) {
    ldmx_log(trace) << "Technical Efficiency plots";
    efficiencyPlots(tracks, measurements, title_);
  }

  // Tagger Recoil Matching

  // Clear the vectors
  ldmx_log(trace) << "Clear the vectors";
  unique_tracks_.clear();
  duplicate_tracks_.clear();
  fake_tracks_.clear();
}

void TrackingRecoDQM::onProcessEnd() {
  // Produce the efficiency plots. (TODO::Switch to TEfficiency instead)
}

void TrackingRecoDQM::efficiencyPlots(
    const std::vector<ldmx::Track>& tracks,
    const std::vector<ldmx::Measurement>& measurements,
    const std::string& title) {
  // Do all truth track plots - denominator

  histograms_.fill(title + "truth_N_tracks", truth_track_collection_->size());
  for (auto& truth_trk : *(truth_track_collection_)) {
    auto truth_phi = truth_trk.getPhi();
    auto truth_d0 = truth_trk.getD0();
    auto truth_z0 = truth_trk.getZ0();
    auto truth_theta = truth_trk.getTheta();
    auto truth_qop = truth_trk.getQoP();
    auto truth_p = 1000. / abs(truth_trk.getQoP());  // MeV
    auto truth_n_hits = truth_trk.getNhits();

    auto truth_mom = truth_trk.getMomentumAtTarget();
    double truth_pt_beam{0.}, truth_beam_angle{0.};
    if (truth_mom.size() == 3) {
      truth_pt_beam =
          std::sqrt(truth_mom[0] * truth_mom[0] + truth_mom[1] * truth_mom[1]);
      truth_beam_angle = std::atan2(truth_pt_beam, truth_mom[2]);
    }

    histograms_.fill(title + "truth_nHits", truth_n_hits);
    histograms_.fill(title + "truth_d0", truth_d0);
    histograms_.fill(title + "truth_z0", truth_z0);
    histograms_.fill(title + "truth_phi", truth_phi);
    histograms_.fill(title + "truth_theta", truth_theta);
    histograms_.fill(title + "truth_qop", truth_qop);
    histograms_.fill(title + "truth_p", truth_p);
    histograms_.fill(title + "truth_beam_angle", truth_beam_angle);

    if (pidmap_.count(truth_trk.getPdgID()) != 0) {
      histograms_.fill(title + "truth_PID", pidmap_[truth_trk.getPdgID()]);

      // TODO do this properly.

      if (pidmap_[truth_trk.getPdgID()] == PIDBins::kminus) {
        histograms_.fill(title + "truth_kminus_p", truth_p);
      }

      if (pidmap_[truth_trk.getPdgID()] == PIDBins::kplus) {
        histograms_.fill(title + "truth_kplus_p", truth_p);
      }

      if (pidmap_[truth_trk.getPdgID()] == PIDBins::piminus) {
        histograms_.fill(title + "truth_piminus_p", truth_p);
      }

      if (pidmap_[truth_trk.getPdgID()] == PIDBins::piplus) {
        histograms_.fill(title + "truth_piplus_p", truth_p);
      }

      if (pidmap_[truth_trk.getPdgID()] == PIDBins::electron) {
        histograms_.fill(title + "truth_electron_p", truth_p);
      }

      if (pidmap_[truth_trk.getPdgID()] == PIDBins::positron) {
        histograms_.fill(title + "truth_positron_p", truth_p);
      }

      if (pidmap_[truth_trk.getPdgID()] == PIDBins::proton) {
        histograms_.fill(title + "truth_proton_p", truth_p);
      }
    }

  }  // loop on truth tracks

  for (auto& track : tracks) {
    // Match the tracks to truth
    ldmx::Track* truth_trk = nullptr;

    auto it = std::find_if(truth_track_collection_->begin(),
                           truth_track_collection_->end(),
                           [&](const ldmx::Track& tt) {
                             return tt.getTrackID() == track.getTrackID();
                           });

    double track_truth_prob = track.getTruthProb();

    if (it != truth_track_collection_->end() &&
        track_truth_prob >= track_prob_cut_)
      truth_trk = &(*it);

    // Match not found
    if (!truth_trk) return;

    auto truth_phi = truth_trk->getPhi();
    auto truth_d0 = truth_trk->getD0();
    auto truth_z0 = truth_trk->getZ0();
    auto truth_theta = truth_trk->getTheta();
    auto truth_qop = truth_trk->getQoP();
    auto truth_p = 1000. / abs(truth_trk->getQoP());  // MeV

    auto truth_mom = truth_trk->getMomentumAtTarget();
    double truth_pt_beam{0.}, truth_beam_angle{0.};
    if (truth_mom.size() == 3) {
      truth_pt_beam =
          std::sqrt(truth_mom[0] * truth_mom[0] + truth_mom[1] * truth_mom[1]);
      truth_beam_angle = std::atan2(truth_pt_beam, truth_mom[2]);
    }

    // Fill reco plots for efficiencies - numerator. The quantities are truth
    histograms_.fill(title + "match_prob", track_truth_prob);
    histograms_.fill(title + "match_d0", truth_d0);
    histograms_.fill(title + "match_z0", truth_z0);
    histograms_.fill(title + "match_phi", truth_phi);
    histograms_.fill(title + "match_theta", truth_theta);
    histograms_.fill(title + "match_p", truth_p);
    histograms_.fill(title + "match_qop", truth_qop);
    histograms_.fill(title + "match_beam_angle", truth_beam_angle);
    histograms_.fill(title + "match_nHits", measurements.size());
    auto dedx_measurements = track.getDedxMeasurements();
    auto measurement_idxs = track.getMeasurementsIdxs();
    for (size_t i = 0; i < measurement_idxs.size(); ++i) {
      histograms_.fill(title + "match_layers_hit",
                       measurements.at(measurement_idxs[i]).getLayer());
      // Add histogram of the measurement dE/dx
      if (i < dedx_measurements.size()) {
        histograms_.fill(title + "match_measurement_dedx",
                         dedx_measurements[i]);
      }
    }

    // For some particles

    if (pidmap_.count(truth_trk->getPdgID()) != 0) {
      histograms_.fill(title + "match_PID", pidmap_[truth_trk->getPdgID()]);

      // TODO do this properly.

      if (pidmap_[truth_trk->getPdgID()] == PIDBins::kminus) {
        histograms_.fill(title + "match_kminus_p", truth_p);
      }

      if (pidmap_[truth_trk->getPdgID()] == PIDBins::kplus) {
        histograms_.fill(title + "match_kplus_p", truth_p);
      }

      if (pidmap_[truth_trk->getPdgID()] == PIDBins::piminus) {
        histograms_.fill(title + "match_piminus_p", truth_p);
      }

      if (pidmap_[truth_trk->getPdgID()] == PIDBins::piplus) {
        histograms_.fill(title + "match_piplus_p", truth_p);
      }

      if (pidmap_[truth_trk->getPdgID()] == PIDBins::electron) {
        histograms_.fill(title + "match_electron_p", truth_p);
      }

      if (pidmap_[truth_trk->getPdgID()] == PIDBins::positron) {
        histograms_.fill(title + "match_positron_p", truth_p);
      }

      if (pidmap_[truth_trk->getPdgID()] == PIDBins::proton) {
        histograms_.fill(title + "match_proton_p", truth_p);
      }
    }
  }  // Loop on tracks

}  // Efficiency plots

void TrackingRecoDQM::trackMonitoring(
    const std::vector<ldmx::Track>& tracks,
    const std::vector<ldmx::Measurement>& measurements, const std::string title,
    const bool& doDetail, const bool& doTruth) {
  for (auto& track : tracks) {
    // Perigee track parameters
    auto trk_d0 = track.getD0();
    auto trk_z0 = track.getZ0();
    auto trk_qop = track.getQoP();
    auto trk_theta = track.getTheta();
    auto trk_phi = track.getPhi();
    auto trk_p = 1000. / abs(trk_qop);  // MeV
    auto dedx_measurements = track.getDedxMeasurements();
    auto measurement_idxs = track.getMeasurementsIdxs();
    for (size_t i = 0; i < measurement_idxs.size(); ++i) {
      histograms_.fill(title + "layers_hit",
                       measurements.at(measurement_idxs[i]).getLayer());
      // Add histogram of the measurement dE/dx
      if (i < dedx_measurements.size()) {
        histograms_.fill(title + "measurement_dedx", dedx_measurements[i]);
      }
    }

    auto trk_mom = track.getMomentumAtTarget();
    // getMomentumAtTarget() returns MeV (LDMX convention)
    double px_ldmx{0.}, py_ldmx{0.}, pz_ldmx{0.};
    double pt_bending{0.}, pt_beam{0.};
    if (trk_mom.size() == 3) {
      px_ldmx = trk_mom[0];  // MeV
      py_ldmx = trk_mom[1];
      pz_ldmx = trk_mom[2];
      // Bending-plane pT: horizontal (x_ldmx) + downstream (z_ldmx)
      pt_bending = std::sqrt(px_ldmx * px_ldmx + pz_ldmx * pz_ldmx);
      // Transverse pT perpendicular to beam: horizontal + vertical
      pt_beam = std::sqrt(px_ldmx * px_ldmx + py_ldmx * py_ldmx);
    }

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
    double sigmap =
        (1000. / trk_qop) * (1000. / trk_qop) * sigmaqop / 1000.;  // MeV

    histograms_.fill(title + "d0", trk_d0);
    histograms_.fill(title + "z0", trk_z0);
    histograms_.fill(title + "qop", trk_qop);
    histograms_.fill(title + "phi", trk_phi);
    histograms_.fill(title + "theta", trk_theta);
    histograms_.fill(title + "p", trk_p);

    if (doDetail) {
      histograms_.fill(title + "px", px_ldmx);
      histograms_.fill(title + "py", py_ldmx);
      histograms_.fill(title + "pz", pz_ldmx);

      histograms_.fill(title + "pt_bending", pt_bending);
      histograms_.fill(title + "pt_beam", pt_beam);

      histograms_.fill(title + "nHits", track.getNhits());
      histograms_.fill(title + "Chi2", track.getChi2());
      histograms_.fill(title + "ndf", track.getNdf());
      histograms_.fill(title + "Chi2_per_ndf",
                       track.getChi2() / track.getNdf());
      histograms_.fill(title + "nShared", track.getNsharedHits());

      histograms_.fill(title + "d0_err", sigmad0);
      histograms_.fill(title + "z0_err", sigmaz0);
      histograms_.fill(title + "phi_err", sigmaphi);
      histograms_.fill(title + "theta_err", sigmatheta);
      histograms_.fill(title + "qop_err", sigmaqop);
      histograms_.fill(title + "p_err", sigmap);

      // 2D Error plots (p in MeV)
      histograms_.fill(title + "d0_err_vs_p", trk_p, sigmad0);
      histograms_.fill(title + "z0_err_vs_p", trk_p, sigmaz0);
      histograms_.fill(title + "p_err_vs_p", trk_p, sigmap);

      if (track.getNhits() == 8)
        histograms_.fill(title + "p_err_vs_p_8hits", trk_p, sigmap);
      else if (track.getNhits() == 9)
        histograms_.fill(title + "p_err_vs_p_9hits", trk_p, sigmap);
      else if (track.getNhits() == 10)
        histograms_.fill(title + "p_err_vs_p_10hits", trk_p, sigmap);
    }

    if (doTruth) {
      // Match to the truth track
      ldmx::Track* truth_trk = nullptr;

      auto it = std::find_if(truth_track_collection_->begin(),
                             truth_track_collection_->end(),
                             [&](const ldmx::Track& tt) {
                               return tt.getTrackID() == track.getTrackID();
                             });

      double track_truth_prob = track.getTruthProb();

      if (it != truth_track_collection_->end() &&
          track_truth_prob >= track_prob_cut_)
        truth_trk = &(*it);

      // Found matched track
      if (truth_trk) {
        auto truth_d0 = truth_trk->getD0();
        auto truth_z0 = truth_trk->getZ0();
        auto truth_phi = truth_trk->getPhi();
        auto truth_theta = truth_trk->getTheta();
        auto truth_qop = truth_trk->getQoP();
        auto truth_p = 1000. / abs(truth_trk->getQoP());  // MeV
        auto truth_mom = truth_trk->getMomentumAtTarget();
        // getMomentumAtTarget() returns MeV (LDMX convention)
        double truth_pt_beam{0.};
        if (truth_mom.size() == 3) {
          truth_pt_beam = std::sqrt(truth_mom[0] * truth_mom[0] +
                                    truth_mom[1] * truth_mom[1]);
        }

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
    }  // do TruthComparison
  }  // loop on tracks

}  // Track Monitoring

void TrackingRecoDQM::trackStateMonitoring(
    const std::vector<ldmx::Track>& tracks, ldmx::TrackStateType ts_type,
    const std::string& ts_title) {
  for (auto& track : tracks) {
    // Match the tracks to truth
    ldmx::Track* truth_trk = nullptr;

    auto it = std::find_if(truth_track_collection_->begin(),
                           truth_track_collection_->end(),
                           [&](const ldmx::Track& tt) {
                             return tt.getTrackID() == track.getTrackID();
                           });

    double track_truth_prob = track.getTruthProb();

    if (it != truth_track_collection_->end() &&
        track_truth_prob >= track_prob_cut_)
      truth_trk = &(*it);

    // Match not found, skip track
    if (!truth_trk) continue;

    auto trk_ts = track.getTrackState(ts_type);
    auto truth_ts = truth_trk->getTrackState(ts_type);

    if (!trk_ts.has_value()) continue;
    if (!truth_ts.has_value()) continue;

    const ldmx::Track::TrackState& target_state = trk_ts.value();
    const ldmx::Track::TrackState& truth_target_state = truth_ts.value();

    // Check that the covariance is filled
    if (target_state.pos_mom_cov_.size() < 21) continue;

    // Sigma from Cartesian position covariance (LDMX frame):
    // pos_mom_cov_ upper-triangle layout: xx=0, yy=6
    double sigmaloc0 = std::sqrt(target_state.pos_mom_cov_[0]);  // sigma_x
    double sigmaloc1 = std::sqrt(target_state.pos_mom_cov_[6]);  // sigma_y

    double trk_qop = track.getQoP();
    double trk_p = 1000. / abs(trk_qop);  // MeV

    // loc0/loc1 = x/y position at the surface in LDMX global frame
    double track_state_loc0 = target_state.pos_[0];
    double track_state_loc1 = target_state.pos_[1];

    double truth_state_loc0 = truth_target_state.pos_[0];
    double truth_state_loc1 = truth_target_state.pos_[1];

    histograms_.fill(title_ + "trk_" + ts_title + "_loc0", track_state_loc0);
    histograms_.fill(title_ + "trk_" + ts_title + "_loc1", track_state_loc1);
    histograms_.fill(title_ + ts_title + "_truth_loc0", truth_state_loc0);
    histograms_.fill(title_ + ts_title + "_truth_loc1", truth_state_loc1);

    // TH1F  residuals
    histograms_.fill(title_ + "trk_" + ts_title + "_loc0-truth_" + ts_title + "_loc0",
                     track_state_loc0 - truth_state_loc0);
    histograms_.fill(title_ + "trk_" + ts_title + "_loc1-truth_" + ts_title + "_loc1",
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
  std::vector<ldmx::Track> sorted_tracks = tracks;

  // Sort the vector of Track objects based on their trackID member
  std::sort(sorted_tracks.begin(), sorted_tracks.end(),
            [](ldmx::Track& t1, ldmx::Track& t2) {
              return t1.getTrackID() < t2.getTrackID();
            });

  // Loop over the sorted vector of Track objects
  for (size_t i = 0; i < sorted_tracks.size(); i++) {
    if (sorted_tracks[i].getTruthProb() < track_prob_cut_)
      fakeTracks.push_back(sorted_tracks[i]);
    else {  // not a fake track
      // If this is the first Track object with this trackID, add it to the
      // uniqueTracks vector directly
      if (uniqueTracks.size() == 0 ||
          sorted_tracks[i].getTrackID() != sorted_tracks[i - 1].getTrackID()) {
        uniqueTracks.push_back(sorted_tracks[i]);
      }
      // Otherwise, add it to the duplicateTracks vector if its truthProb is
      // lower than the existing Track object Otherwise, if the truthProbability
      // is higher than the track stored in uniqueTracks, put it in uniqueTracks
      // and move the uniqueTracks.back to duplicateTracks.
      else if (sorted_tracks[i].getTruthProb() >
               uniqueTracks.back().getTruthProb()) {
        duplicateTracks.push_back(uniqueTracks.back());
        uniqueTracks.back() = sorted_tracks[i];
      }
      // Otherwise, add it to the duplicateTracks vector
      else {
        duplicateTracks.push_back(sorted_tracks[i]);
      }
    }  // a real track
  }  // loop on sorted tracks
  // The total number of elements in the uniqueTracks and duplicateTracks
  // vectors should be equal to the number of elements in the original tracks
  // vector
  if (uniqueTracks.size() + duplicateTracks.size() + fakeTracks.size() !=
      tracks.size()) {
    std::cerr << "Error: unique and duplicate tracks vectors do not add up to "
                 "original tracks vector";
    return;
  }

  // Iterate through the uniqueTracks vector and duplicateTracks vector
  ldmx_log(trace) << "Unique tracks:";
  for (const ldmx::Track& track : uniqueTracks) {
    ldmx_log(trace) << "\tTrack ID: " << track.getTrackID()
                    << ", Truth Prob: " << track.getTruthProb();
  }
  ldmx_log(trace) << "Duplicate tracks:";
  for (const ldmx::Track& track : duplicateTracks) {
    ldmx_log(trace) << "\tTrack ID: " << track.getTrackID()
                    << ", Truth Prob: " << track.getTruthProb();
  }
  ldmx_log(trace) << "Fake tracks:";
  for (const ldmx::Track& track : fakeTracks) {
    ldmx_log(trace) << "\tTrack ID: " << track.getTrackID()
                    << ", Truth Prob: " << track.getTruthProb();
  }
}
}  // namespace tracking::dqm

DECLARE_ANALYZER(tracking::dqm::TrackingRecoDQM)

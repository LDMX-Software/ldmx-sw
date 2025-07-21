#include "Ecal/EcalVetoProcessor.h"

namespace ecal {

void EcalVetoProcessor::onNewRun(const ldmx::RunHeader &rh) {
  profiling_map_["setup"] = 0.;
  profiling_map_["recoil_electron"] = 0.;
  profiling_map_["trajectories"] = 0.;
  profiling_map_["roc_var"] = 0.;
  profiling_map_["fill_hitmaps"] = 0.;
  profiling_map_["containment_var"] = 0.;
  profiling_map_["mip_tracking_setup"] = 0.;
  profiling_map_["set_variables"] = 0.;
  profiling_map_["bdt_variables"] = 0.;
}

void EcalVetoProcessor::buildBDTFeatureVector(
    const ldmx::EcalVetoResult &result) {
  // Base variables
  bdt_features_.push_back(result.getNReadoutHits());
  bdt_features_.push_back(result.getSummedDet());
  bdt_features_.push_back(result.getSummedTightIso());
  bdt_features_.push_back(result.getMaxCellDep());
  bdt_features_.push_back(result.getShowerRMS());
  bdt_features_.push_back(result.getXStd());
  bdt_features_.push_back(result.getYStd());
  bdt_features_.push_back(result.getAvgLayerHit());
  bdt_features_.push_back(result.getStdLayerHit());
  bdt_features_.push_back(result.getDeepestLayerHit());
  bdt_features_.push_back(result.getEcalBackEnergy());
  // MIP tracking

  bdt_features_.push_back(-1.);  // NStraight
  bdt_features_.push_back(-1.);  // FirstNearPHLayer
  bdt_features_.push_back(-1.);  // NNearPHHits
  bdt_features_.push_back(-1.);  // PhotonTerritoryHits

  // bdt_features_.push_back(result.getNStraightTracks());
  // bdt_features_.push_back(result.getFirstNearPhLayer());
  // bdt_features_.push_back(result.getNNearPhHits());
  // bdt_features_.push_back(result.getPhotonTerritoryHits());
  // bdt_features_.push_back(result.getNTrackingHits());
  bdt_features_.push_back(result.getEPSep());
  bdt_features_.push_back(result.getEPDot());
  // Longitudinal segment variables
  bdt_features_.push_back(result.getEnergySeg()[0]);
  bdt_features_.push_back(result.getXMeanSeg()[0]);
  bdt_features_.push_back(result.getYMeanSeg()[0]);
  bdt_features_.push_back(result.getLayerMeanSeg()[0]);
  bdt_features_.push_back(result.getEnergySeg()[1]);
  bdt_features_.push_back(result.getYMeanSeg()[2]);
  /// Electron RoC variables
  bdt_features_.push_back(result.getEleContEnergy()[0][0]);
  bdt_features_.push_back(result.getEleContEnergy()[1][0]);
  bdt_features_.push_back(result.getEleContYMean()[0][0]);
  bdt_features_.push_back(result.getEleContEnergy()[0][1]);
  bdt_features_.push_back(result.getEleContEnergy()[1][1]);
  bdt_features_.push_back(result.getEleContYMean()[0][1]);
  /// Photon RoC variables
  bdt_features_.push_back(result.getPhContNHits()[0][0]);
  bdt_features_.push_back(result.getPhContYMean()[0][0]);
  bdt_features_.push_back(result.getPhContNHits()[0][1]);
  /// Outside RoC variables
  bdt_features_.push_back(result.getOutContEnergy()[0][0]);
  bdt_features_.push_back(result.getOutContEnergy()[1][0]);
  bdt_features_.push_back(result.getOutContEnergy()[2][0]);
  bdt_features_.push_back(result.getOutContNHits()[0][0]);
  bdt_features_.push_back(result.getOutContXMean()[0][0]);
  bdt_features_.push_back(result.getOutContYMean()[0][0]);
  bdt_features_.push_back(result.getOutContYMean()[1][0]);
  bdt_features_.push_back(result.getOutContYStd()[0][0]);
  bdt_features_.push_back(result.getOutContEnergy()[0][1]);
  bdt_features_.push_back(result.getOutContEnergy()[1][1]);
  bdt_features_.push_back(result.getOutContEnergy()[2][1]);
  bdt_features_.push_back(result.getOutContLayerMean()[0][1]);
  bdt_features_.push_back(result.getOutContLayerStd()[0][1]);
  bdt_features_.push_back(result.getOutContEnergy()[0][2]);
  bdt_features_.push_back(result.getOutContLayerMean()[0][2]);
}

void EcalVetoProcessor::configure(framework::config::Parameters &parameters) {
  feature_list_name_ = parameters.get<std::string>("feature_list_name");

  sim_particles_passname_ =
      parameters.get<std::string>("sim_particles_passname");
  // Load BDT ONNX file
  rt_ = std::make_unique<ldmx::Ort::ONNXRuntime>(
      parameters.get<std::string>("bdt_file"));

  // Read in arrays holding 68% containment radius per layer
  // for different bins in momentum/angle
  roc_file_name_ = parameters.get<std::string>("roc_file");
  if (!std::ifstream(roc_file_name_).good()) {
    EXCEPTION_RAISE(
        "EcalVetoProcessor",
        "The specified RoC file '" + roc_file_name_ + "' does not exist!");
  } else {
    std::ifstream rocfile(roc_file_name_);
    std::string line, value;

    // Extract the first line in the file
    std::getline(rocfile, line);
    std::vector<float> values;

    // Read data, line by line
    while (std::getline(rocfile, line)) {
      std::stringstream ss(line);
      values.clear();
      while (std::getline(ss, value, ',')) {
        float f_value = (value != "") ? std::stof(value) : -1.0;
        values.push_back(f_value);
      }
      roc_range_values_.push_back(values);
    }
  }

  n_ecal_layers_ = parameters.get<int>("num_ecal_layers");

  bdt_cut_val_ = parameters.get<double>("disc_cut");
  ecal_layer_edep_raw_.resize(n_ecal_layers_, 0);
  ecal_layer_edep_readout_.resize(n_ecal_layers_, 0);
  ecal_layer_time_.resize(n_ecal_layers_, 0);

  beam_energy_mev_ = parameters.get<double>("beam_energy");
  // Set the collection name as defined in the configuration
  sp_pass_name_ = parameters.get<std::string>("sp_pass_name");
  collection_name_ = parameters.get<std::string>("collection_name");
  rec_pass_name_ = parameters.get<std::string>("rec_pass_name");
  rec_coll_name_ = parameters.get<std::string>("rec_coll_name");
  recoil_from_tracking_ = parameters.get<bool>("recoil_from_tracking");
  track_collection_ = parameters.get<std::string>("track_collection");
  track_pass_name_ = parameters.get<std::string>("track_pass_name", "");
  inverse_skim_ = parameters.get<bool>("inverse_skim");
}

void EcalVetoProcessor::clearProcessor() {
  cell_map_.clear();
  cell_map_tight_iso_.clear();
  bdt_features_.clear();

  n_readout_hits_ = 0;
  summed_det_ = 0;
  summed_tight_iso_ = 0;
  max_cell_dep_ = 0;
  shower_rms_ = 0;
  x_std_ = 0;
  y_std_ = 0;
  avg_layer_hit_ = 0;
  std_layer_hit_ = 0;
  deepest_layer_hit_ = 0;
  ecal_back_energy_ = 0;
  n_tracking_hits_ = 0;
  ep_ang_ = 0;
  ep_ang_at_target_ = 0;
  ep_sep_ = 0;
  ep_dot_ = 0;
  ep_dot_at_target_ = 0;

  std::fill(ecal_layer_edep_raw_.begin(), ecal_layer_edep_raw_.end(), 0);
  std::fill(ecal_layer_edep_readout_.begin(), ecal_layer_edep_readout_.end(),
            0);
  std::fill(ecal_layer_time_.begin(), ecal_layer_time_.end(), 0);
}

void EcalVetoProcessor::produce(framework::Event &event) {
  auto start = std::chrono::high_resolution_clock::now();
  nevents_++;

  // Get the Ecal Geometry
  geometry_ = &getCondition<ldmx::EcalGeometry>(
      ldmx::EcalGeometry::CONDITIONS_OBJECT_NAME);

  ldmx::EcalVetoResult result;

  clearProcessor();

  // Get the collection of Ecal scoring plane hits. If it doesn't exist,
  // don't bother adding any truth tracking information.

  std::array<float, 3> recoil_p = {0., 0., 0.};
  std::array<float, 3> recoil_pos = {-9999., -9999., -9999.};
  std::array<float, 3> recoil_p_at_target = {0., 0., 0.};
  std::array<float, 3> recoil_pos_at_target = {-9999., -9999., -9999.};

  auto setup = std::chrono::high_resolution_clock::now();
  profiling_map_["setup"] +=
      std::chrono::duration<float, std::milli>(setup - start).count();

  if (!recoil_from_tracking_ &&
      event.exists("EcalScoringPlaneHits", sp_pass_name_)) {
    ldmx_log(trace) << "   Loop through all of the sim particles and find the "
                       "recoil electron";
    //
    // Loop through all of the sim particles and find the recoil electron.
    //

    // Get the collection of simulated particles from the event
    auto particle_map{event.getMap<int, ldmx::SimParticle>(
        "SimParticles", sim_particles_passname_)};

    // Search for the recoil electron
    auto [recoil_track_id, recoil_electron] = Analysis::getRecoil(particle_map);

    // Find ECAL SP hit for recoil electron
    auto ecal_sp_hits{event.getCollection<ldmx::SimTrackerHit>(
        "EcalScoringPlaneHits", sp_pass_name_)};
    float pmax = 0;
    for (ldmx::SimTrackerHit &spHit : ecal_sp_hits) {
      ldmx::SimSpecialID hit_id(spHit.getID());
      auto ecal_sp_momentum = spHit.getMomentum();
      auto ecal_sp_position = spHit.getPosition();
      if (hit_id.plane() != 31 || ecal_sp_momentum[2] <= 0) continue;

      if (spHit.getTrackID() == recoil_track_id) {
        // A*A is faster than pow(A,2)
        if (sqrt((ecal_sp_momentum[0] * ecal_sp_momentum[0]) +
                 (ecal_sp_momentum[1] * ecal_sp_momentum[1]) +
                 (ecal_sp_momentum[2] * ecal_sp_momentum[2])) > pmax) {
          recoil_p = {static_cast<float>(ecal_sp_momentum[0]),
                      static_cast<float>(ecal_sp_momentum[1]),
                      static_cast<float>(ecal_sp_momentum[2])};
          recoil_pos = {(ecal_sp_position[0]), (ecal_sp_position[1]),
                        (ecal_sp_position[2])};
          pmax = sqrt(recoil_p[0] * recoil_p[0] + recoil_p[1] * recoil_p[1] +
                      recoil_p[2] * recoil_p[2]);
        }
      }
    }

    // Find target SP hit for recoil electron
    if (event.exists("TargetScoringPlaneHits", sp_pass_name_)) {
      std::vector<ldmx::SimTrackerHit> target_sp_hits =
          event.getCollection<ldmx::SimTrackerHit>("TargetScoringPlaneHits",
                                                   sp_pass_name_);
      pmax = 0;
      for (ldmx::SimTrackerHit &spHit : target_sp_hits) {
        ldmx::SimSpecialID hit_id(spHit.getID());
        auto target_sp_momentum = spHit.getMomentum();
        auto target_sp_position = spHit.getPosition();
        if (hit_id.plane() != 1 || target_sp_momentum[2] <= 0) continue;

        if (spHit.getTrackID() == recoil_track_id) {
          if (sqrt((target_sp_momentum[0] * target_sp_momentum[0]) +
                   (target_sp_momentum[1] * target_sp_momentum[1]) +
                   (target_sp_momentum[2] * target_sp_momentum[2])) > pmax) {
            recoil_p_at_target = {static_cast<float>(target_sp_momentum[0]),
                                  static_cast<float>(target_sp_momentum[1]),
                                  static_cast<float>(target_sp_momentum[2])};
            recoil_pos_at_target = {target_sp_position[0],
                                    target_sp_position[1],
                                    target_sp_position[2]};
            // (A*A) is faster than pow(A,2)
            pmax = sqrt((recoil_p_at_target[0] * recoil_p_at_target[0]) +
                        (recoil_p_at_target[1] * recoil_p_at_target[1]) +
                        (recoil_p_at_target[2] * recoil_p_at_target[2]));
          }
        }
      }  // end loop on target SP hits
    }  // end condition on target SP
  }  // end condition on ecal SP

  // Get recoil_pos using recoil tracking
  bool fiducial_in_tracker{false};
  if (recoil_from_tracking_) {
    ldmx_log(trace) << "  Get recoil tracks collection";

    // Get the recoil track collection
    auto recoil_tracks{
        event.getCollection<ldmx::Track>(track_collection_, track_pass_name_)};

    ldmx_log(trace) << "  Propagate the recoil ele to the ECAL";
    ldmx::TrackStateType ts_type = ldmx::TrackStateType::AtECAL;
    auto recoil_track_states_ecal =
        ecal::trackProp(recoil_tracks, ts_type, "ecal");
    ldmx_log(trace) << "  Propagate the recoil ele to the Target";
    ldmx::TrackStateType ts_type_target = ldmx::TrackStateType::AtTarget;
    auto recoil_track_states_target =
        ecal::trackProp(recoil_tracks, ts_type_target, "target");

    ldmx_log(trace) << "  Set recoil_pos and recoil_p";
    // Redefining recoil_pos now to come from the track state
    // track_state_loc0 is recoil_pos[0] and track_state_loc1 is recoil_pos[1]
    if (!recoil_track_states_ecal.empty()) {
      fiducial_in_tracker = true;
      recoil_pos = {recoil_track_states_ecal[0], recoil_track_states_ecal[1],
                    recoil_track_states_ecal[2]};
      recoil_p = {(recoil_track_states_ecal[3]), (recoil_track_states_ecal[4]),
                  (recoil_track_states_ecal[5])};
    } else {
      ldmx_log(trace) << "  No recoil track at ECAL";
      fiducial_in_tracker = false;
    }
    ldmx_log(debug) << "  Set recoil_p = (" << recoil_p[0] << ", "
                    << recoil_p[1] << ", " << recoil_p[2]
                    << ") and recoil_pos = (" << recoil_pos[0] << ", "
                    << recoil_pos[1] << ", " << recoil_pos[2] << ")";

    // Repeat the above but now for the taget states
    if (!recoil_track_states_target.empty()) {
      recoil_pos_at_target = {(recoil_track_states_target[0]),
                              (recoil_track_states_target[1]),
                              (recoil_track_states_target[2])};
      recoil_p_at_target = {recoil_track_states_target[3],
                            recoil_track_states_target[4],
                            recoil_track_states_target[5]};
    }
    ldmx_log(debug) << "  Set recoil_p_at_target = (" << recoil_p_at_target[0]
                    << ", " << recoil_p_at_target[1] << ", "
                    << recoil_p_at_target[2] << ") and recoil_pos_at_target = ("
                    << recoil_pos_at_target[0] << ", "
                    << recoil_pos_at_target[1] << ", "
                    << recoil_pos_at_target[2] << ")";
  }  // condition to do recoil information from tracking

  ldmx_log(trace) << "   Get projected trajectories for electron and photon";

  auto recoil_electron = std::chrono::high_resolution_clock::now();
  profiling_map_["recoil_electron"] +=
      std::chrono::duration<float, std::milli>(recoil_electron - setup).count();

  // Get projected trajectories for electron and photon
  std::vector<XYCoords> ele_trajectory, photon_trajectory,
      ele_trajectory_at_target;
  // Require that z-momentum is positive (which will also exclude the default
  // initializaton) Require that the positions are not the default initializaton
  if ((recoil_p[2] > 0.) && (recoil_p_at_target[2] > 0.) &&
      (recoil_pos[0] != -9999.) && (recoil_pos_at_target[0] != -9999.)) {
    ele_trajectory = getTrajectory(recoil_p, recoil_pos);
    // Get the photon projection. This does not require that the photon exists
    // tho
    std::array<float, 3> photon_proj_momentum = {
        -recoil_p_at_target[0], -recoil_p_at_target[1],
        beam_energy_mev_ - recoil_p_at_target[2]};
    photon_trajectory =
        getTrajectory(photon_proj_momentum, recoil_pos_at_target);
  } else {
    ldmx_log(trace) << "Ele / photon trajectory cannot be determined, pZ = "
                    << recoil_p[2] << " pZAtTarget = " << recoil_p_at_target[2]
                    << " X = " << recoil_pos[0]
                    << " XAtTarget = " << recoil_pos_at_target[0];
  }

  float recoil_p_mag = (recoil_p[2] > 0.) ? sqrt((recoil_p[0] * recoil_p[0]) +
                                                 (recoil_p[1] * recoil_p[1]) +
                                                 (recoil_p[2] * recoil_p[2]))
                                          : -1.0;
  float recoil_theta =
      recoil_p_mag > 0 ? acos(recoil_p[2] / recoil_p_mag) * 180.0 / M_PI : -1.0;

  ldmx_log(trace) << "   Build Radii of containment (ROC)";

  auto trajectories = std::chrono::high_resolution_clock::now();
  profiling_map_["trajectories"] +=
      std::chrono::duration<float, std::milli>(trajectories - recoil_electron)
          .count();

  // Use the appropriate containment radii for the recoil electron
  std::vector<float> roc_values_bin_0(roc_range_values_[0].begin() + 4,
                                      roc_range_values_[0].end());
  std::vector<float> ele_radii = roc_values_bin_0;
  float theta_min, theta_max, p_min, p_max;
  bool inrange;

  for (int i = 0; i < roc_range_values_.size(); i++) {
    theta_min = roc_range_values_[i][0];
    theta_max = roc_range_values_[i][1];
    p_min = roc_range_values_[i][2];
    p_max = roc_range_values_[i][3];
    inrange = true;

    if (theta_min != -1.0) {
      inrange = inrange && (recoil_theta >= theta_min);
    }
    if (theta_max != -1.0) {
      inrange = inrange && (recoil_theta < theta_max);
    }
    if (p_min != -1.0) {
      inrange = inrange && (recoil_p_mag >= p_min);
    }
    if (p_max != -1.0) {
      inrange = inrange && (recoil_p_mag < p_max);
    }
    if (inrange) {
      std::vector<float> roc_values_bini(roc_range_values_[i].begin() + 4,
                                         roc_range_values_[i].end());
      ele_radii = roc_values_bini;
    }
  }
  // Use default RoC bin for photon
  std::vector<float> photon_radii = roc_values_bin_0;

  auto roc_var = std::chrono::high_resolution_clock::now();
  profiling_map_["roc_var"] +=
      std::chrono::duration<float, std::milli>(roc_var - trajectories).count();

  // Get the collection of digitized Ecal hits from the event.
  const std::vector<ldmx::EcalHit> ecal_rec_hits =
      event.getCollection<ldmx::EcalHit>(rec_coll_name_, rec_pass_name_);

  ldmx::EcalID global_centroid =
      GetShowerCentroidIDAndRMS(ecal_rec_hits, shower_rms_);
  /* ~~ Fill the hit map ~~ O(n)  */
  fillHitMap(ecal_rec_hits, cell_map_);
  bool do_tight = true;
  /* ~~ Fill the isolated hit maps ~~ O(n)  */
  fillIsolatedHitMap(ecal_rec_hits, global_centroid, cell_map_,
                     cell_map_tight_iso_, do_tight);

  auto fill_hitmaps = std::chrono::high_resolution_clock::now();
  profiling_map_["fill_hitmaps"] +=
      std::chrono::duration<float, std::milli>(fill_hitmaps - roc_var).count();

  // Loop over the hits from the event to calculate the rest of the important
  // quantities

  float w_avg_layer_hit = 0;
  float x_mean = 0;
  float y_mean = 0;

  // Containment variables
  unsigned int n_regions = 5;
  std::vector<float> electron_containment_energy(n_regions, 0.0);
  std::vector<float> photon_containment_energy(n_regions, 0.0);
  std::vector<float> outside_containment_energy(n_regions, 0.0);
  std::vector<int> outside_containment_n_hits(n_regions, 0);
  std::vector<float> outside_containment_x_mean(n_regions, 0.0);
  std::vector<float> outside_containment_y_mean(n_regions, 0.0);
  std::vector<float> outside_containment_x_std(n_regions, 0.0);
  std::vector<float> outside_containment_y_std(n_regions, 0.0);
  // Longitudinal segmentation
  std::vector<int> seg_layers = {0, 6, 17, 34};
  unsigned int n_segments = seg_layers.size() - 1;
  std::vector<float> energy_seg(n_segments, 0.0);
  std::vector<float> x_mean_seg(n_segments, 0.0);
  std::vector<float> x_std_seg(n_segments, 0.0);
  std::vector<float> y_mean_seg(n_segments, 0.0);
  std::vector<float> y_std_seg(n_segments, 0.0);
  std::vector<float> layer_mean_seg(n_segments, 0.0);
  std::vector<float> layer_std_seg(n_segments, 0.0);
  std::vector<std::vector<float>> e_cont_energy(
      n_regions, std::vector<float>(n_segments, 0.0));
  std::vector<std::vector<float>> e_cont_x_mean(
      n_regions, std::vector<float>(n_segments, 0.0));
  std::vector<std::vector<float>> e_cont_y_mean(
      n_regions, std::vector<float>(n_segments, 0.0));
  std::vector<std::vector<float>> g_cont_energy(
      n_regions, std::vector<float>(n_segments, 0.0));
  std::vector<std::vector<int>> g_cont_n_hits(n_regions,
                                              std::vector<int>(n_segments, 0));
  std::vector<std::vector<float>> g_cont_x_mean(
      n_regions, std::vector<float>(n_segments, 0.0));
  std::vector<std::vector<float>> g_cont_y_mean(
      n_regions, std::vector<float>(n_segments, 0.0));
  std::vector<std::vector<float>> o_cont_energy(
      n_regions, std::vector<float>(n_segments, 0.0));
  std::vector<std::vector<int>> o_cont_n_hits(n_regions,
                                              std::vector<int>(n_segments, 0));
  std::vector<std::vector<float>> o_cont_x_mean(
      n_regions, std::vector<float>(n_segments, 0.0));
  std::vector<std::vector<float>> o_cont_y_mean(
      n_regions, std::vector<float>(n_segments, 0.0));
  std::vector<std::vector<float>> o_cont_x_std(
      n_regions, std::vector<float>(n_segments, 0.0));
  std::vector<std::vector<float>> o_cont_y_std(
      n_regions, std::vector<float>(n_segments, 0.0));
  std::vector<std::vector<float>> o_cont_layer_mean(
      n_regions, std::vector<float>(n_segments, 0.0));
  std::vector<std::vector<float>> o_cont_layer_std(
      n_regions, std::vector<float>(n_segments, 0.0));

  auto containment_var = std::chrono::high_resolution_clock::now();
  profiling_map_["containment_var"] +=
      std::chrono::duration<float, std::milli>(containment_var - fill_hitmaps)
          .count();

  // MIP tracking:  vector of hits to be used in the MIP tracking algorithm. All
  // hits inside the electron ROC (or all hits in the ECal if the event is
  // missing an electron) will be included.
  std::vector<ldmx::HitData> tracking_hit_list;

  ldmx_log(trace)
      << "   Loop over the hits from the event to calculate the BDT features";

  for (const ldmx::EcalHit &hit : ecal_rec_hits) {
    // Layer-wise quantities
    ldmx::EcalID id(hit.getID());
    ecal_layer_edep_raw_[id.layer()] =
        ecal_layer_edep_raw_[id.layer()] + hit.getEnergy();
    if (id.layer() >= 20) ecal_back_energy_ += hit.getEnergy();
    if (max_cell_dep_ < hit.getEnergy()) max_cell_dep_ = hit.getEnergy();
    if (hit.getEnergy() <= 0) {
      ldmx_log(fatal)
          << "ECal hit has negative or zero energy, this should never happen, "
             "check the threshold settings in HgcrocEmulator";
      continue;
    }
    n_readout_hits_++;
    ecal_layer_edep_readout_[id.layer()] += hit.getEnergy();
    ecal_layer_time_[id.layer()] += (hit.getEnergy()) * hit.getTime();
    auto [x, y, z] = geometry_->getPosition(id);
    x_mean += x * hit.getEnergy();
    y_mean += y * hit.getEnergy();
    avg_layer_hit_ += id.layer();
    w_avg_layer_hit += id.layer() * hit.getEnergy();
    if (deepest_layer_hit_ < id.layer()) {
      deepest_layer_hit_ = id.layer();
    }
    XYCoords xy_pair = std::make_pair(x, y);
    float distance_ele_trajectory =
        ele_trajectory.size()
            ? sqrt(pow((xy_pair.first - ele_trajectory[id.layer()].first), 2) +
                   pow((xy_pair.second - ele_trajectory[id.layer()].second), 2))
            : -1.0;
    float distance_photon_trajectory =
        photon_trajectory.size()
            ? sqrt(pow((xy_pair.first - photon_trajectory[id.layer()].first),
                       2) +
                   pow((xy_pair.second - photon_trajectory[id.layer()].second),
                       2))
            : -1.0;

    // Decide which longitudinal segment the hit is in and add to sums
    for (unsigned int iseg = 0; iseg < n_segments; iseg++) {
      if (id.layer() >= seg_layers[iseg] &&
          id.layer() <= seg_layers[iseg + 1] - 1) {
        energy_seg[iseg] += hit.getEnergy();
        x_mean_seg[iseg] += xy_pair.first * hit.getEnergy();
        y_mean_seg[iseg] += xy_pair.second * hit.getEnergy();
        layer_mean_seg[iseg] += id.layer() * hit.getEnergy();

        // Decide which containment region the hit is in and add to sums
        for (unsigned int ireg = 0; ireg < n_regions; ireg++) {
          if (distance_ele_trajectory >= ireg * ele_radii[id.layer()] &&
              distance_ele_trajectory < (ireg + 1) * ele_radii[id.layer()]) {
            e_cont_energy[ireg][iseg] += hit.getEnergy();
            e_cont_x_mean[ireg][iseg] += xy_pair.first * hit.getEnergy();
            e_cont_y_mean[ireg][iseg] += xy_pair.second * hit.getEnergy();
          }
          if (distance_photon_trajectory >= ireg * photon_radii[id.layer()] &&
              distance_photon_trajectory <
                  (ireg + 1) * photon_radii[id.layer()]) {
            g_cont_energy[ireg][iseg] += hit.getEnergy();
            g_cont_n_hits[ireg][iseg] += 1;
            g_cont_x_mean[ireg][iseg] += xy_pair.first * hit.getEnergy();
            g_cont_y_mean[ireg][iseg] += xy_pair.second * hit.getEnergy();
          }
          if (distance_ele_trajectory > (ireg + 1) * ele_radii[id.layer()] &&
              distance_photon_trajectory >
                  (ireg + 1) * photon_radii[id.layer()]) {
            o_cont_energy[ireg][iseg] += hit.getEnergy();
            o_cont_n_hits[ireg][iseg] += 1;
            o_cont_x_mean[ireg][iseg] += xy_pair.first * hit.getEnergy();
            o_cont_y_mean[ireg][iseg] += xy_pair.second * hit.getEnergy();
            o_cont_layer_mean[ireg][iseg] += id.layer() * hit.getEnergy();
          }
        }
      }
    }

    // Decide which containment region the hit is in and add to sums
    for (unsigned int ireg = 0; ireg < n_regions; ireg++) {
      if (distance_ele_trajectory >= ireg * ele_radii[id.layer()] &&
          distance_ele_trajectory < (ireg + 1) * ele_radii[id.layer()])
        electron_containment_energy[ireg] += hit.getEnergy();
      if (distance_photon_trajectory >= ireg * photon_radii[id.layer()] &&
          distance_photon_trajectory < (ireg + 1) * photon_radii[id.layer()])
        photon_containment_energy[ireg] += hit.getEnergy();
      if (distance_ele_trajectory > (ireg + 1) * ele_radii[id.layer()] &&
          distance_photon_trajectory > (ireg + 1) * photon_radii[id.layer()]) {
        outside_containment_energy[ireg] += hit.getEnergy();
        outside_containment_n_hits[ireg] += 1;
        outside_containment_x_mean[ireg] += xy_pair.first * hit.getEnergy();
        outside_containment_y_mean[ireg] += xy_pair.second * hit.getEnergy();
      }
    }

    // MIP tracking:  Decide whether hit should be added to tracking_hit_list
    // If inside e- RoC or if etraj is missing, use the hit for tracking:
    if (distance_ele_trajectory >= ele_radii[id.layer()] ||
        distance_ele_trajectory == -1.0) {
      ldmx::HitData hd;
      hd.pos = TVector3(xy_pair.first, xy_pair.second,
                        geometry_->getZPosition(id.layer()));
      hd.layer = id.layer();
      tracking_hit_list.push_back(hd);
    }
  }  // end loop over rechits

  n_tracking_hits_ = tracking_hit_list.size();

  for (const auto &[id, energy] : cell_map_tight_iso_) {
    if (energy > 0) summed_tight_iso_ += energy;
  }

  for (int iLayer = 0; iLayer < ecal_layer_edep_readout_.size(); iLayer++) {
    ecal_layer_time_[iLayer] =
        ecal_layer_time_[iLayer] / ecal_layer_edep_readout_[iLayer];
    summed_det_ += ecal_layer_edep_readout_[iLayer];
  }

  if (n_readout_hits_ > 0) {
    avg_layer_hit_ /= n_readout_hits_;
    w_avg_layer_hit /= summed_det_;
    x_mean /= summed_det_;
    y_mean /= summed_det_;
  } else {
    w_avg_layer_hit = 0;
    avg_layer_hit_ = 0;
    x_mean = 0;
    y_mean = 0;
  }

  // If necessary, quotient out the total energy from the means
  for (unsigned int iseg = 0; iseg < n_segments; iseg++) {
    if (energy_seg[iseg] > 0) {
      x_mean_seg[iseg] /= energy_seg[iseg];
      y_mean_seg[iseg] /= energy_seg[iseg];
      layer_mean_seg[iseg] /= energy_seg[iseg];
    }
    for (unsigned int ireg = 0; ireg < n_regions; ireg++) {
      if (e_cont_energy[ireg][iseg] > 0) {
        e_cont_x_mean[ireg][iseg] /= e_cont_energy[ireg][iseg];
        e_cont_y_mean[ireg][iseg] /= e_cont_energy[ireg][iseg];
      }
      if (g_cont_energy[ireg][iseg] > 0) {
        g_cont_x_mean[ireg][iseg] /= g_cont_energy[ireg][iseg];
        g_cont_y_mean[ireg][iseg] /= g_cont_energy[ireg][iseg];
      }
      if (o_cont_energy[ireg][iseg] > 0) {
        o_cont_x_mean[ireg][iseg] /= o_cont_energy[ireg][iseg];
        o_cont_y_mean[ireg][iseg] /= o_cont_energy[ireg][iseg];
        o_cont_layer_mean[ireg][iseg] /= o_cont_energy[ireg][iseg];
      }
    }
  }

  for (unsigned int ireg = 0; ireg < n_regions; ireg++) {
    if (outside_containment_energy[ireg] > 0) {
      outside_containment_x_mean[ireg] /= outside_containment_energy[ireg];
      outside_containment_y_mean[ireg] /= outside_containment_energy[ireg];
    }
  }

  // Loop over hits a second time to find the standard deviations.
  for (const ldmx::EcalHit &hit : ecal_rec_hits) {
    ldmx::EcalID id(hit.getID());
    auto [x, y, z] = geometry_->getPosition(id);
    if (hit.getEnergy() > 0) {
      x_std_ += pow((x - x_mean), 2) * hit.getEnergy();
      y_std_ += pow((y - y_mean), 2) * hit.getEnergy();
      std_layer_hit_ +=
          pow((id.layer() - w_avg_layer_hit), 2) * hit.getEnergy();
    }
    XYCoords xy_pair = std::make_pair(x, y);
    float distance_ele_trajectory =
        ele_trajectory.size()
            ? sqrt(pow((xy_pair.first - ele_trajectory[id.layer()].first), 2) +
                   pow((xy_pair.second - ele_trajectory[id.layer()].second), 2))
            : -1.0;
    float distance_photon_trajectory =
        photon_trajectory.size()
            ? sqrt(pow((xy_pair.first - photon_trajectory[id.layer()].first),
                       2) +
                   pow((xy_pair.second - photon_trajectory[id.layer()].second),
                       2))
            : -1.0;

    for (unsigned int iseg = 0; iseg < n_segments; iseg++) {
      if (id.layer() >= seg_layers[iseg] &&
          id.layer() <= seg_layers[iseg + 1] - 1) {
        x_std_seg[iseg] +=
            pow((xy_pair.first - x_mean_seg[iseg]), 2) * hit.getEnergy();
        y_std_seg[iseg] +=
            pow((xy_pair.second - y_mean_seg[iseg]), 2) * hit.getEnergy();
        layer_std_seg[iseg] +=
            pow((id.layer() - layer_mean_seg[iseg]), 2) * hit.getEnergy();

        for (unsigned int ireg = 0; ireg < n_regions; ireg++) {
          if (distance_ele_trajectory > (ireg + 1) * ele_radii[id.layer()] &&
              distance_photon_trajectory >
                  (ireg + 1) * photon_radii[id.layer()]) {
            o_cont_x_std[ireg][iseg] +=
                pow((xy_pair.first - o_cont_x_mean[ireg][iseg]), 2) *
                hit.getEnergy();
            o_cont_y_std[ireg][iseg] +=
                pow((xy_pair.second - o_cont_y_mean[ireg][iseg]), 2) *
                hit.getEnergy();
            o_cont_layer_std[ireg][iseg] +=
                pow((id.layer() - o_cont_layer_mean[ireg][iseg]), 2) *
                hit.getEnergy();
          }
        }
      }
    }

    for (unsigned int ireg = 0; ireg < n_regions; ireg++) {
      if (distance_ele_trajectory > (ireg + 1) * ele_radii[id.layer()] &&
          distance_photon_trajectory > (ireg + 1) * photon_radii[id.layer()]) {
        outside_containment_x_std[ireg] +=
            pow((xy_pair.first - outside_containment_x_mean[ireg]), 2) *
            hit.getEnergy();
        outside_containment_y_std[ireg] +=
            pow((xy_pair.second - outside_containment_y_mean[ireg]), 2) *
            hit.getEnergy();
      }
    }
  }  // end loop over rechits (2nd time)

  if (n_readout_hits_ > 0) {
    x_std_ = sqrt(x_std_ / summed_det_);
    y_std_ = sqrt(y_std_ / summed_det_);
    std_layer_hit_ = sqrt(std_layer_hit_ / summed_det_);
  } else {
    x_std_ = 0;
    y_std_ = 0;
    std_layer_hit_ = 0;
  }

  // Quotient out the total energies from the standard deviations if possible
  // and take root
  for (unsigned int iseg = 0; iseg < n_segments; iseg++) {
    if (energy_seg[iseg] > 0) {
      x_std_seg[iseg] = sqrt(x_std_seg[iseg] / energy_seg[iseg]);
      y_std_seg[iseg] = sqrt(y_std_seg[iseg] / energy_seg[iseg]);
      layer_std_seg[iseg] = sqrt(layer_std_seg[iseg] / energy_seg[iseg]);
    }
    for (unsigned int ireg = 0; ireg < n_regions; ireg++) {
      if (o_cont_energy[ireg][iseg] > 0) {
        o_cont_x_std[ireg][iseg] =
            sqrt(o_cont_x_std[ireg][iseg] / o_cont_energy[ireg][iseg]);
        o_cont_y_std[ireg][iseg] =
            sqrt(o_cont_y_std[ireg][iseg] / o_cont_energy[ireg][iseg]);
        o_cont_layer_std[ireg][iseg] =
            sqrt(o_cont_layer_std[ireg][iseg] / o_cont_energy[ireg][iseg]);
      }
    }
  }

  for (unsigned int ireg = 0; ireg < n_regions; ireg++) {
    if (outside_containment_energy[ireg] > 0) {
      outside_containment_x_std[ireg] = sqrt(outside_containment_x_std[ireg] /
                                             outside_containment_energy[ireg]);
      outside_containment_y_std[ireg] = sqrt(outside_containment_y_std[ireg] /
                                             outside_containment_energy[ireg]);
    }
  }

  ldmx_log(trace) << "   Find out if the recoil electron is fiducial";

  // Find the location of the recoil electron
  // Ecal face is not where the first layer starts,
  // defined in DetDescr/python/EcalGeometry.py
  const float dz_from_face{7.932};
  float drifted_recoil_x{-9999.};
  float drifted_recoil_y{-9999.};
  if (recoil_p[2] > 0.) {
    ldmx_log(trace) << "   Recoil electron pX = " << recoil_p[0]
                    << " pY = " << recoil_p[1] << " pZ = " << recoil_p[2];
    ldmx_log(trace) << "   Recoil electron PosX = " << recoil_pos[0]
                    << " PosY = " << recoil_pos[1]
                    << " PosZ = " << recoil_pos[2];
    drifted_recoil_x =
        (dz_from_face * (recoil_p[0] / recoil_p[2])) + recoil_pos[0];
    drifted_recoil_y =
        (dz_from_face * (recoil_p[1] / recoil_p[2])) + recoil_pos[1];
  }
  const int recoil_layer_index = 0;

  // Check if it's fiducial
  bool inside_ecal_cell{false};
  // At module level
  const auto ecal_id = geometry_->getID(drifted_recoil_x, drifted_recoil_y,
                                        recoil_layer_index, true);
  if (!ecal_id.null()) {
    // If fiducial at module level, check at cell level
    const auto cell_id =
        geometry_->getID(drifted_recoil_x, drifted_recoil_y, recoil_layer_index,
                         ecal_id.getModuleID(), true);
    if (!cell_id.null()) {
      inside_ecal_cell = true;
    }
  }

  ldmx_log(info) << "   Is this event is fiducial in ECAL? "
                 << inside_ecal_cell;
  TVector3 e_traj_start;
  TVector3 e_traj_end;
  TVector3 e_traj_target_start;
  TVector3 e_traj_target_end;
  TVector3 p_traj_start;
  TVector3 p_traj_end;
  if (!ele_trajectory.empty() && !photon_trajectory.empty()) {
    // Create TVector3s marking the start and endpoints of each projected
    // trajectory
    e_traj_start.SetXYZ(ele_trajectory[0].first, ele_trajectory[0].second,
                        geometry_->getZPosition(0));
    e_traj_end.SetXYZ(ele_trajectory[(n_ecal_layers_ - 1)].first,
                      ele_trajectory[(n_ecal_layers_ - 1)].second,
                      geometry_->getZPosition((n_ecal_layers_ - 1)));
    p_traj_start.SetXYZ(photon_trajectory[0].first, photon_trajectory[0].second,
                        geometry_->getZPosition(0));
    p_traj_end.SetXYZ(photon_trajectory[(n_ecal_layers_ - 1)].first,
                      photon_trajectory[(n_ecal_layers_ - 1)].second,
                      geometry_->getZPosition((n_ecal_layers_ - 1)));

    TVector3 evec = e_traj_end - e_traj_start;
    TVector3 e_norm = evec.Unit();
    TVector3 pvec = p_traj_end - p_traj_start;
    TVector3 p_norm = pvec.Unit();

    // Calculate the angle between the projected electron at Ecal and the photon
    // (at target)
    ep_dot_ = e_norm.Dot(p_norm);
    ep_ang_ = acos(ep_dot_) * 180.0 / M_PI;
    ep_sep_ = sqrt(pow(e_traj_start.X() - p_traj_start.X(), 2) +
                   pow(e_traj_start.Y() - p_traj_start.Y(), 2));
    // Calculate the electron trajectory with positions and momentum as measured
    // at the target
    if (!ele_trajectory_at_target.empty()) {
      e_traj_target_start.SetXYZ(ele_trajectory_at_target[0].first,
                                 ele_trajectory_at_target[0].second,
                                 geometry_->getZPosition(0));
      e_traj_target_end.SetXYZ(ele_trajectory_at_target[(0)].first,
                               ele_trajectory_at_target[(0)].second,
                               geometry_->getZPosition((n_ecal_layers_ - 1)));
      // Now calculate the ep angle at the target
      TVector3 evec_target = e_traj_target_end - e_traj_target_start;
      TVector3 e_norm_target = evec_target.Unit();
      ep_dot_at_target_ = e_norm_target.Dot(p_norm);
      ep_ang_at_target_ = acos(ep_dot_at_target_) * 180.0 / M_PI;
    }
    ldmx_log(trace) << "   Electron trajectory calculated";
  } else {
    // Electron trajectory is missing, so all hits in the Ecal are fair game.
    // Pick e/ptraj so that they won't restrict the tracking algorithm (place
    // them far outside the ECal).
    ldmx_log(trace) << "   Electron trajectory is missing";
    e_traj_start = TVector3(999, 999, geometry_->getZPosition(0));
    e_traj_end =
        TVector3(999, 999, geometry_->getZPosition((n_ecal_layers_ - 1)));
    p_traj_start = TVector3(1000, 1000, geometry_->getZPosition(0));
    p_traj_end =
        TVector3(1000, 1000, geometry_->getZPosition((n_ecal_layers_ - 1)));
    /*ensures event will not be vetoed by angle/separation cut */
    ep_ang_ = 999.;
    ep_ang_at_target_ = 999.;
    ep_sep_ = 999.;
    ep_dot_ = 999.;
    ep_dot_at_target_ = 999.;
  }
  // Took out MIP tracking here (starting at near photon hits)
  ldmx::EcalTrajectoryInfo ecal_mip_collection;
  ldmx_log(trace) << "   Set up input info  for MIP tracking";
  ecal_mip_collection.setEleTrajectory(ele_trajectory);
  ecal_mip_collection.setPhotonTrajectory(photon_trajectory);
  ecal_mip_collection.setTrackingHitList(tracking_hit_list);
  event.add("EcalTrajectoryInfo", ecal_mip_collection);

  auto mip_tracking_setup = std::chrono::high_resolution_clock::now();
  profiling_map_["mip_tracking_setup"] +=
      std::chrono::duration<double, std::milli>(mip_tracking_setup - start)
          .count();
  result.setVariables(
      n_readout_hits_, deepest_layer_hit_, n_tracking_hits_, summed_det_,
      summed_tight_iso_, max_cell_dep_, shower_rms_, x_std_, y_std_,
      avg_layer_hit_, std_layer_hit_, ecal_back_energy_, ep_ang_,
      ep_ang_at_target_, ep_sep_, ep_dot_, ep_dot_at_target_,
      electron_containment_energy, photon_containment_energy,
      outside_containment_energy, outside_containment_n_hits,
      outside_containment_x_std, outside_containment_y_std, energy_seg,
      x_mean_seg, y_mean_seg, x_std_seg, y_std_seg, layer_mean_seg,
      layer_std_seg, e_cont_energy, e_cont_x_mean, e_cont_y_mean, g_cont_energy,
      g_cont_n_hits, g_cont_x_mean, g_cont_y_mean, o_cont_energy, o_cont_n_hits,
      o_cont_x_mean, o_cont_y_mean, o_cont_x_std, o_cont_y_std,
      o_cont_layer_mean, o_cont_layer_std, ecal_layer_edep_readout_, recoil_p,
      recoil_pos);

  auto set_variables = std::chrono::high_resolution_clock::now();
  profiling_map_["set_variables"] += std::chrono::duration<double, std::milli>(
                                         set_variables - mip_tracking_setup)
                                         .count();
  buildBDTFeatureVector(result);
  ldmx::Ort::FloatArrays inputs({bdt_features_});
  float pred =
      rt_->run({feature_list_name_}, inputs, {"probabilities"})[0].at(1);
  ldmx_log(info) << " BDT was ran, score is " << pred;
  // Other considerations were (nLinregTracks_ == 0)  && (firstNearPhLayer_ >=
  // 6)
  // && (ep_ang_ > 3.0 && ep_ang_ < 900 || ep_sep_ > 10.0 && ep_sep_ < 900)
  result.setVetoResult(pred > bdt_cut_val_);
  result.setDiscValue(pred);
  ldmx_log(info) << " The pred > bdtCutVal = " << (pred > bdt_cut_val_);

  // Persist in the event if the recoil ele is fiducial
  result.setFiducial(inside_ecal_cell);
  result.setTrackingFiducial(fiducial_in_tracker);

  // If the event passes the veto, keep it. Otherwise,
  // drop the event.
  if (!inverse_skim_) {
    if (result.passesVeto()) {
      setStorageHint(framework::hint_shouldKeep);
    } else {
      setStorageHint(framework::hint_shouldDrop);
    }
  } else {
    // Invert the skimming logic
    if (result.passesVeto()) {
      setStorageHint(framework::hint_shouldDrop);
    } else {
      setStorageHint(framework::hint_shouldKeep);
    }
  }

  event.add(collection_name_, result);

  auto bdt_variables = std::chrono::high_resolution_clock::now();
  profiling_map_["bdt_variables"] +=
      std::chrono::duration<float, std::milli>(bdt_variables - set_variables)
          .count();

  auto end = std::chrono::high_resolution_clock::now();
  auto time_diff = end - start;
  processing_time_ +=
      std::chrono::duration<float, std::milli>(time_diff).count();
}

void EcalVetoProcessor::onProcessEnd() {
  ldmx_log(info) << "AVG Time/Event: " << std::fixed << std::setprecision(2)
                 << processing_time_ / nevents_ << " ms";

  ldmx_log(info) << "Breakdown::";

  ldmx_log(info) << "setup                  Avg Time/Event = " << std::fixed
                 << std::setprecision(3) << profiling_map_["setup"] / nevents_
                 << " ms";

  ldmx_log(info) << "recoil_electron        Avg Time/Event = " << std::fixed
                 << std::setprecision(3)
                 << profiling_map_["recoil_electron"] / nevents_ << " ms";
  ldmx_log(info) << "trajectories           Avg Time/Event = " << std::fixed
                 << std::setprecision(3)
                 << profiling_map_["trajectories"] / nevents_ << " ms";

  ldmx_log(info) << "roc_var                Avg Time/Event = " << std::fixed
                 << std::setprecision(3) << profiling_map_["roc_var"] / nevents_
                 << " ms";

  ldmx_log(info) << "fill_hitmaps           Avg Time/Event = " << std::fixed
                 << std::setprecision(3)
                 << profiling_map_["fill_hitmaps"] / nevents_ << " ms";
  ldmx_log(info) << "containment_var        Avg Time/Event = " << std::fixed
                 << std::setprecision(3)
                 << profiling_map_["containment_var"] / nevents_ << " ms";
  ldmx_log(info) << "mip_tracking_setup     Avg Time/Event = " << std::fixed
                 << std::setprecision(3)
                 << profiling_map_["mip_tracking_setup"] / nevents_ << " ms";

  ldmx_log(info) << "set_variables           Avg Time/Event = " << std::fixed
                 << std::setprecision(3)
                 << profiling_map_["set_variables"] / nevents_ << " ms";

  ldmx_log(info) << "bdt_variables           Avg Time/Event = " << std::fixed
                 << std::setprecision(3)
                 << profiling_map_["bdt_variables"] / nevents_ << " ms";
}
/* Function to calculate the energy weighted shower centroid */
ldmx::EcalID EcalVetoProcessor::GetShowerCentroidIDAndRMS(
    const std::vector<ldmx::EcalHit> &ecal_rec_hits, float &shower_rms) {
  auto wgt_centroid_coords = std::make_pair<float, float>(0., 0.);
  float sumEdep = 0;
  ldmx::EcalID returnCellId;

  // Calculate Energy Weighted Centroid
  for (const ldmx::EcalHit &hit : ecal_rec_hits) {
    ldmx::EcalID id(hit.getID());
    CellEnergyPair cell_energy_pair = std::make_pair(id, hit.getEnergy());
    auto [x, y, z] = geometry_->getPosition(id);
    XYCoords centroidCoords = std::make_pair(x, y);
    wgt_centroid_coords.first = wgt_centroid_coords.first +
                                centroidCoords.first * cell_energy_pair.second;
    wgt_centroid_coords.second =
        wgt_centroid_coords.second +
        centroidCoords.second * cell_energy_pair.second;
    sumEdep += cell_energy_pair.second;
  }
  wgt_centroid_coords.first = (sumEdep > 1E-6)
                                  ? wgt_centroid_coords.first / sumEdep
                                  : wgt_centroid_coords.first;
  wgt_centroid_coords.second = (sumEdep > 1E-6)
                                   ? wgt_centroid_coords.second / sumEdep
                                   : wgt_centroid_coords.second;
  // Find Nearest Cell to Centroid
  float maxDist = 1e6;
  for (const ldmx::EcalHit &hit : ecal_rec_hits) {
    auto [x, y, z] = geometry_->getPosition(hit.getID());
    XYCoords centroidCoords = std::make_pair(x, y);

    float deltaR =
        pow(pow((centroidCoords.first - wgt_centroid_coords.first), 2) +
                pow((centroidCoords.second - wgt_centroid_coords.second), 2),
            .5);
    shower_rms += deltaR * hit.getEnergy();
    if (deltaR < maxDist) {
      maxDist = deltaR;
      returnCellId = ldmx::EcalID(hit.getID());
    }
  }
  if (sumEdep > 0) shower_rms = shower_rms / sumEdep;
  // flatten
  return ldmx::EcalID(0, returnCellId.module(), returnCellId.cell());
}

/**
 * Function to load up empty vector of hit maps
 */
void EcalVetoProcessor::fillHitMap(
    const std::vector<ldmx::EcalHit> &ecal_rec_hits,
    std::map<ldmx::EcalID, float> &cellMap) {
  for (const ldmx::EcalHit &hit : ecal_rec_hits) {
    ldmx::EcalID id(hit.getID());
    cellMap.emplace(id, hit.getEnergy());
  }
}

void EcalVetoProcessor::fillIsolatedHitMap(
    const std::vector<ldmx::EcalHit> &ecal_rec_hits,
    ldmx::EcalID global_centroid, std::map<ldmx::EcalID, float> &cellMap,
    std::map<ldmx::EcalID, float> &cellMapIso, bool do_tight) {
  for (const ldmx::EcalHit &hit : ecal_rec_hits) {
    auto isolatedHit = std::make_pair(true, ldmx::EcalID());
    ldmx::EcalID id(hit.getID());
    if (do_tight) {
      // Disregard hits that are on the centroid.
      if (id == global_centroid) continue;

      // Skip hits that are on centroid inner ring
      if (geometry_->isNN(global_centroid, id)) {
        continue;
      }
    }

    // Skip hits that have a readout neighbor
    // Get neighboring cell id's and try to look them up in the full cell map
    // (constant speed algo.)
    //  these ideas are only cell/module (must ignore layer)
    std::vector<ldmx::EcalID> cellNbrIds = geometry_->getNN(id);

    for (int k = 0; k < cellNbrIds.size(); k++) {
      // update neighbor ID to the current layer
      cellNbrIds[k] = ldmx::EcalID(id.layer(), cellNbrIds[k].module(),
                                   cellNbrIds[k].cell());
      // look in cell hit map to see if it is there
      if (cellMap.find(cellNbrIds[k]) != cellMap.end()) {
        isolatedHit = std::make_pair(false, cellNbrIds[k]);
        break;
      }
    }
    if (!isolatedHit.first) {
      continue;
    }
    // Insert isolated hit
    cellMapIso.emplace(id, hit.getEnergy());
  }
}

/* Calculate where trajectory intersects ECAL layers using position and momentum
 * at scoring plane */
std::vector<std::pair<float, float>> EcalVetoProcessor::getTrajectory(
    std::array<float, 3> momentum, std::array<float, 3> position) {
  std::vector<XYCoords> positions;
  for (int iLayer = 0; iLayer < n_ecal_layers_; iLayer++) {
    float posX =
        position[0] + (momentum[0] / momentum[2]) *
                          (geometry_->getZPosition(iLayer) - position[2]);
    float posY =
        position[1] + (momentum[1] / momentum[2]) *
                          (geometry_->getZPosition(iLayer) - position[2]);
    positions.push_back(std::make_pair(posX, posY));
  }
  return positions;
}

// MIP tracking functions:

float EcalVetoProcessor::distTwoLines(TVector3 v1, TVector3 v2, TVector3 w1,
                                      TVector3 w2) {
  TVector3 e1 = v1 - v2;
  TVector3 e2 = w1 - w2;
  TVector3 crs = e1.Cross(e2);
  if (crs.Mag() == 0) {
    return 100.0;  // arbitrary large number; edge case that shouldn't cause
                   // problems.
  } else {
    return std::abs(crs.Dot(v1 - w1) / crs.Mag());
  }
}

float EcalVetoProcessor::distPtToLine(TVector3 h1, TVector3 p1, TVector3 p2) {
  return ((h1 - p1).Cross(h1 - p2)).Mag() / (p1 - p2).Mag();
}

}  // namespace ecal

DECLARE_PRODUCER(ecal::EcalVetoProcessor);

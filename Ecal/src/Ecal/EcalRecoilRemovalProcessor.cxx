#include "Ecal/EcalRecoilRemovalProcessor.h"

namespace ecal {

void EcalRecoilRemovalProcessor::onNewRun(const ldmx::RunHeader& rh) {
  profiling_map_["setup"] = 0.;
  profiling_map_["recoil_electron"] = 0.;
  profiling_map_["trajectories"] = 0.;
  profiling_map_["rem_dist"] = 0.;
  profiling_map_["recoil_removal"] = 0.;
}

void EcalRecoilRemovalProcessor::onProcessEnd() {
  ldmx_log(info) << "Total Avg Time/Event: " << std::fixed
                 << std::setprecision(2) << processing_time_ / nevents_
                 << " ms";
  ldmx_log(info) << "Breakdown::";

  for (const auto& [key, value] : profiling_map_) {
    ldmx_log(info) << std::left << std::setw(20) << key
                   << "Avg Time/Event = " << std::fixed << std::setprecision(3)
                   << value / nevents_ << " ms";
  }
}

void EcalRecoilRemovalProcessor::configure(
    framework::config::Parameters& parameters) {
  beam_energy_mev_ = parameters.get<double>("beam_energy");
  num_ecal_layers_ = parameters.get<int>("num_ecal_layers");
  rem_dist_file_name_ = parameters.get<std::string>("rem_dist_file");
  collection_name_included_ =
      parameters.get<std::string>("collection_name_included");
  collection_name_excluded_ =
      parameters.get<std::string>("collection_name_excluded");
  rec_coll_name_ = parameters.get<std::string>("rec_coll_name");
  rec_pass_name_ = parameters.get<std::string>("rec_pass_name");
  ecal_sp_hits_pass_name_ =
      parameters.get<std::string>("ecal_sp_hits_pass_name");
  ecal_sim_pass_name_ = parameters.get<std::string>("ecal_sim_pass_name");
  recoil_from_tracking_ = parameters.get<bool>("recoil_from_tracking");
  track_coll_name_ = parameters.get<std::string>("track_coll_name");
  track_pass_name_ = parameters.get<std::string>("track_pass_name");
  n_electrons_ = parameters.get<int>(
      "n_electrons");  // number of electrons in the event; TODO: replace with
                       // the ElectronCounter processor result

  // Read in array holding the removal distances for Ecal hit removals
  if (!std::ifstream(rem_dist_file_name_).good()) {
    EXCEPTION_RAISE("EcalRecoilRemovalProcessor",
                    "The specified removal distances file '" +
                        rem_dist_file_name_ + "' does not exist!");
  } else {
    std::ifstream remdistfile(rem_dist_file_name_);
    std::string line, value;

    // Extract the first line in the file
    std::getline(remdistfile, line);
    std::vector<float> values;

    // Read data, line by line
    while (std::getline(remdistfile, line)) {
      std::stringstream ss(line);
      values.clear();
      while (std::getline(ss, value, ',')) {
        float f_value = (value != "") ? std::stof(value) : -1.0;
        values.push_back(f_value);
      }
      rem_dist_values_.push_back(values);
    }
  }

  if (!recoil_from_tracking_) {
    EXCEPTION_RAISE("EcalRecoilRemovalProcessor",
                    "The processor is currently not configured to use sim "
                    "information! Please set recoil_from_tracking = True");
  }
}

void EcalRecoilRemovalProcessor::produce(framework::Event& event) {
  ///////////////////////////////////////////////////////
  //////////////////////// SETUP ////////////////////////
  ///////////////////////////////////////////////////////
  auto start = std::chrono::high_resolution_clock::now();
  nevents_++;

  // Get the Ecal Geometry
  geometry_ = &getCondition<ldmx::EcalGeometry>(
      ldmx::EcalGeometry::CONDITIONS_OBJECT_NAME);

  std::vector<std::array<float, 3>> ele_p;
  std::vector<std::array<float, 3>> ele_pos;
  std::vector<bool> fiducial_in_tracker;

  auto setup_finish = std::chrono::high_resolution_clock::now();
  profiling_map_["setup"] +=
      std::chrono::duration<float, std::milli>(setup_finish - start).count();

  ///////////////////////////////////////////////////
  ///////////////// RECOIL ELECTRON /////////////////
  ///////////////////////////////////////////////////

  // TODO: Gear this for multiple electron tracks in the Ecal, right now it can
  // only handle one
  // if (!recoil_from_tracking_ &&
  //     event.exists("EcalScoringPlaneHits", ecal_sp_hits_pass_name_) &&
  //     n_electrons_ == 1) {
  //   ldmx_log(trace) << "    Loop through all of the sim particles and find
  //   the "
  //                      "recoil electron";

  //   // Get the collection of simulated particles from the event
  //   auto particle_map{event.getMap<int, ldmx::SimParticle>(
  //       "SimParticles", ecal_sim_pass_name_)};

  //   // Loop through all of the sim particles and find the recoil electron.
  //   auto [recoil_track_id, recoil_electron] =
  //   analysis::getRecoil(particle_map);

  //   // Find ECAL SP hit for recoil electron
  //   auto ecal_sp_hits{event.getCollection<ldmx::SimTrackerHit>(
  //       "EcalScoringPlaneHits", ecal_sp_hits_pass_name_)};
  //   float pmax = 0;
  //   for (ldmx::SimTrackerHit &sp_hit : ecal_sp_hits) {
  //     ldmx::SimSpecialID hit_id(sp_hit.getID());
  //     auto ecal_sp_momentum = sp_hit.getMomentum();
  //     auto ecal_sp_position = sp_hit.getPosition();
  //     if (hit_id.plane() != 31 || ecal_sp_momentum[2] <= 0) continue;

  //     if (sp_hit.getTrackID() == recoil_track_id) {
  //       // A*A is faster than pow(A,2)
  //       if (sqrt((ecal_sp_momentum[0] * ecal_sp_momentum[0]) +
  //                (ecal_sp_momentum[1] * ecal_sp_momentum[1]) +
  //                (ecal_sp_momentum[2] * ecal_sp_momentum[2])) > pmax) {
  //         recoil_p = {static_cast<float>(ecal_sp_momentum[0]),
  //                     static_cast<float>(ecal_sp_momentum[1]),
  //                     static_cast<float>(ecal_sp_momentum[2])};
  //         recoil_pos = {(ecal_sp_position[0]), (ecal_sp_position[1]),
  //                       (ecal_sp_position[2])};
  //         pmax = sqrt(recoil_p[0] * recoil_p[0] + recoil_p[1] * recoil_p[1] +
  //                     recoil_p[2] * recoil_p[2]);
  //         ldmx_log(debug) << "    Set recoil_p = (" << recoil_p[0] << ", "
  //                         << recoil_p[1] << ", " << recoil_p[2]
  //                         << ") and recoil_pos = (" << recoil_pos[0] << ", "
  //                         << recoil_pos[1] << ", " << recoil_pos[2] << ")";
  //       }
  //     }
  //   }
  // } else if (!event.exists(
  //                "EcalScoringPlaneHits",
  //                ecal_sp_hits_pass_name_)) {  // end condition on ecal SP
  //   ldmx_log(debug)
  //       << "Event does not exist in collection EcalScoringPlaneHits";
  // }

  // Get recoil_pos using recoil tracking
  if (recoil_from_tracking_) {
    ldmx_log(trace) << "    Get recoil tracks collection";

    // Get the recoil track collection
    auto recoil_tracks{
        event.getCollection<ldmx::Track>(track_coll_name_, track_pass_name_)};

    ldmx_log(trace) << "    Propagate the recoil ele to the ECAL";
    auto ele_track_states =  // std::vector<std::vector<float>> OR empty vector
        ecal::pTTrackProp(recoil_tracks, n_electrons_);
    if (!ele_track_states.empty()) {
      for (int i = 0; i < ele_track_states.size(); ++i) {
        std::vector<float>& recoil_track_states_ecal = ele_track_states[i];
        std::array<float, 3> recoil_pos;
        std::array<float, 3> recoil_p;
        // track_state_loc0 is recoil_pos[0] and track_state_loc1 is
        // recoil_pos[1]
        if (!recoil_track_states_ecal.empty()) {
          recoil_pos = {recoil_track_states_ecal[0],
                        recoil_track_states_ecal[1],
                        recoil_track_states_ecal[2]};
          recoil_p = {(recoil_track_states_ecal[3]),
                      (recoil_track_states_ecal[4]),
                      (recoil_track_states_ecal[5])};
          fiducial_in_tracker.push_back(true);
        } else {
          recoil_pos = {-9999.f, -9999.f, -9999.f};
          recoil_p = {0.f, 0.f, 0.f};
          fiducial_in_tracker.push_back(false);
          ldmx_log(info) << "    Electron trajectory is empty!";
        }
        ldmx_log(debug) << "    Electron " << i + 1 << ": set recoil_p = ("
                        << recoil_p[0] << ", " << recoil_p[1] << ", "
                        << recoil_p[2] << ") and recoil_pos = ("
                        << recoil_pos[0] << ", " << recoil_pos[1] << ", "
                        << recoil_pos[2] << ")";
        ele_p.push_back(recoil_p);
        ele_pos.push_back(recoil_pos);
      }  // end loop on electron track states
    } else {  // end condition on nonempty electron track states
      ldmx_log(info) << "    No valid electron tracks found in recoil tracking "
                        "information";
    }
  }  // end condition to do recoil information from tracking

  auto recoil_electron_finish = std::chrono::high_resolution_clock::now();
  profiling_map_["recoil_electron"] +=
      std::chrono::duration<float, std::milli>(recoil_electron_finish -
                                               setup_finish)
          .count();

  ////////////////////////////////////////////////
  ///////////////// TRAJECTORIES /////////////////
  ////////////////////////////////////////////////

  ldmx_log(trace) << "    Get projected trajectories for electron and photon";

  std::vector<std::vector<XYCoords>> ele_trajectories;
  std::vector<float> ele_p_mag;
  std::vector<float> ele_theta;

  if (!ele_p.empty() && !ele_pos.empty()) {
    for (int i = 0; i < ele_p.size(); ++i) {
      std::array<float, 3>& recoil_p = ele_p[i];
      std::array<float, 3>& recoil_pos = ele_pos[i];
      std::vector<XYCoords> ele_trajectory;

      // Require that z-momentum is positive (which will also exclude the
      // default initializaton) Require that the positions are not the default
      // initializaton
      if ((recoil_p[2] > 0.) && (recoil_pos[0] != -9999.)) {
        ele_trajectory = getTrajectory(recoil_p, recoil_pos);
      } else {
        ldmx_log(trace) << "Ele trajectory cannot be determined, pZ = "
                        << recoil_p[2] << " X = " << recoil_pos[0];
      }

      // calculate removal distance binning variables
      float recoil_p_mag =
          (recoil_p[2] > 0.)
              ? sqrt((recoil_p[0] * recoil_p[0]) + (recoil_p[1] * recoil_p[1]) +
                     (recoil_p[2] * recoil_p[2]))
              : -1.0;
      float recoil_theta = recoil_p_mag > 0
                               ? acos(recoil_p[2] / recoil_p_mag) * 180.0 / M_PI
                               : -1.0;

      // push back variable values
      ele_trajectories.push_back(ele_trajectory);
      ele_p_mag.push_back(recoil_p_mag);
      ele_theta.push_back(recoil_theta);

    }  // end loop on track states
  }  // end condition on ele_p and ele_pos emptiness

  auto trajectories_finish = std::chrono::high_resolution_clock::now();
  profiling_map_["trajectories"] +=
      std::chrono::duration<float, std::milli>(trajectories_finish -
                                               recoil_electron_finish)
          .count();

  ///////////////////////////////////////////////////////////
  ///////////////// REMOVAL DISTANCES SETUP /////////////////
  ///////////////////////////////////////////////////////////

  ldmx_log(trace) << "    Build recoil removal distances vector";
  std::vector<float> rem_dist_values_bin_0(rem_dist_values_[0].begin() + 4,
                                           rem_dist_values_[0].end());
  std::vector<std::vector<float>> removal_distances;

  if (!ele_trajectories.empty()) {
    for (int k = 0; k < ele_p_mag.size(); ++k) {
      float theta_min, theta_max, p_min, p_max;
      bool inrange;
      std::vector<float> rec_rem_dists = rem_dist_values_bin_0;
      float& recoil_p_mag = ele_p_mag[k];
      float& recoil_theta = ele_theta[k];

      // Use the appropriate containment radii for the recoil electron
      for (int i = 0; i < rem_dist_values_.size(); i++) {
        theta_min = rem_dist_values_[i][0];
        theta_max = rem_dist_values_[i][1];
        p_min = rem_dist_values_[i][2];
        p_max = rem_dist_values_[i][3];
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
          std::vector<float> rem_dist_values_bini(
              rem_dist_values_[i].begin() + 4, rem_dist_values_[i].end());
          rec_rem_dists = rem_dist_values_bini;
        }
      }
      removal_distances.push_back(rec_rem_dists);
    }  // end loop on ele_trajectories
  }  // end condition on nonempty ele_trajectories

  auto rem_dist_finish = std::chrono::high_resolution_clock::now();
  profiling_map_["rem_dist"] += std::chrono::duration<float, std::milli>(
                                    rem_dist_finish - trajectories_finish)
                                    .count();

  //////////////////////////////////////////////////////////
  ///////////////// REMOVE RECOIL ELECTRON /////////////////
  //////////////////////////////////////////////////////////

  // Get the collection of digitized Ecal hits from the event.
  const std::vector<ldmx::EcalHit> ecal_rec_hits =
      event.getCollection<ldmx::EcalHit>(rec_coll_name_, rec_pass_name_);

  std::vector<ldmx::EcalHit> ecal_rec_hits_inc;
  std::vector<ldmx::EcalHit> ecal_rec_hits_exc;

  // the time complexity of this should just be O(n_ecal_rec_hits *
  // n_ele_trajectories) if I've done things right
  if (!ele_trajectories.empty()) {
    ldmx_log(trace) << "      ======== EcalRecHitInc List (length"
                    << ecal_rec_hits_inc.size() << ") ========";
    for (const ldmx::EcalHit& hit :
         ecal_rec_hits) {  // loops through reconstructed hits in the ecal and
                           // discards all those inside the electron's RoC
      ldmx::EcalID id(hit.getID());
      auto [x, y, z] = geometry_->getPosition(id);
      XYCoords xy_pair = std::make_pair(x, y);

      bool include = true;
      for (int i = 0; i < ele_trajectories.size(); ++i) {
        std::vector<XYCoords>& ele_trajectory = ele_trajectories[i];
        std::vector<float>& rec_rem_dists = removal_distances[i];

        float dist_ele_traj =  // calculates distance between hit location and
                               // projected particle positions
            ele_trajectory.size()
                ? sqrt(pow((xy_pair.first - ele_trajectory[id.layer()].first),
                           2) +
                       pow((xy_pair.second - ele_trajectory[id.layer()].second),
                           2))
                : -1.0;
        if (dist_ele_traj == -1.0) {
          ldmx_log(trace) << "        ele_trajectory does not exist; KEEP";
          continue;
        } else if (dist_ele_traj <=
                   (rec_rem_dists[id.layer()])) {  // if the hit is inside some
                                                   // distance of the electron,
                                                   // discard it; else keep it
          ldmx_log(trace) << "        dist_ele_traj = " << dist_ele_traj
                          << " <= " << rec_rem_dists[id.layer()] << "; DISCARD";
          include = false;
        } else {
          ldmx_log(trace) << "        dist_ele_traj = " << dist_ele_traj
                          << " > " << rec_rem_dists[id.layer()] << "; KEEP";
          continue;
        }
      }  // end loop on electron trajectories

      // drop or keep hit
      if (include) {
        ecal_rec_hits_inc.emplace_back(hit);
      } else {
        ecal_rec_hits_exc.emplace_back(hit);
      }

    }  // end loop on ecal_rec_hits
    ldmx_log(trace) << "      ======== END OF ecal_rec_hit List ========";
  } else {  // end condition on nonempty ele_trajectories
    ecal_rec_hits_inc = ecal_rec_hits;
  }
  ldmx_log(info) << "    Removed " << ecal_rec_hits_exc.size()
                 << " hits within recoil electron RoC; "
                 << ecal_rec_hits_inc.size() << " hits remaining of "
                 << ecal_rec_hits.size();

  // creates collection for reduced set of rec hits for analysis
  event.add(collection_name_included_, ecal_rec_hits_inc);
  // creates collection of discarded rec hits
  event.add(collection_name_excluded_, ecal_rec_hits_exc);

  auto recoil_removal_finish = std::chrono::high_resolution_clock::now();
  profiling_map_["recoil_removal"] +=
      std::chrono::duration<float, std::milli>(recoil_removal_finish -
                                               rem_dist_finish)
          .count();

  auto end = std::chrono::high_resolution_clock::now();
  auto time_diff = end - start;
  processing_time_ +=
      std::chrono::duration<float, std::milli>(time_diff).count();

}  // end EcalRecoilRemovalProcessor::produce

/* Calculate where trajectory intersects ECAL layers using position and
 * momentum at scoring plane */
std::vector<ldmx::XYCoords> EcalRecoilRemovalProcessor::getTrajectory(
    std::array<float, 3> momentum, std::array<float, 3> position) {
  std::vector<XYCoords> positions;
  for (int i_layer = 0; i_layer < num_ecal_layers_; i_layer++) {
    float pos_x =
        position[0] + (momentum[0] / momentum[2]) *
                          (geometry_->getZPosition(i_layer) - position[2]);
    float pos_y =
        position[1] + (momentum[1] / momentum[2]) *
                          (geometry_->getZPosition(i_layer) - position[2]);
    positions.push_back(std::make_pair(pos_x, pos_y));
  }
  return positions;
}  // end EcalRecoilRemovalProcessor::getTrajectory

}  // end namespace ecal

DECLARE_PRODUCER(ecal::EcalRecoilRemovalProcessor);
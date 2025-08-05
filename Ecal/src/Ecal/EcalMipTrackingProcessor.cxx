#include "Ecal/EcalMipTrackingProcessor.h"

namespace ecal {

void EcalMipTrackingProcessor::onNewRun(const ldmx::RunHeader &rh) {
  profiling_map_["straight_tracks"] = 0.;
  profiling_map_["linreg_tracks"] = 0.;
  profiling_map_["processing_time_"] = 0;
}

void EcalMipTrackingProcessor::configure(
    framework::config::Parameters &parameters) {
  n_ecal_layers_ = parameters.getParameter<int>("num_ecal_layers");
  linreg_radius_ = parameters.getParameter<double>("linreg_radius");
  ecal_collection_name_ =
      parameters.getParameter<std::string>("ecal_collection_name");
  ecal_pass_name_ = parameters.getParameter<std::string>("ecal_pass_name");
  mip_collection_name_ =
      parameters.getParameter<std::string>("mip_collection_name");
  mip_pass_name_ = parameters.getParameter<std::string>("mip_pass_name");
  mip_result_name_ = parameters.getParameter<std::string>("mip_result_name");
}

void EcalMipTrackingProcessor::clearProcessor() {
  // MIP tracking
  n_straight_tracks_ = 0;
  n_linreg_tracks_ = 0;
  first_near_ph_layer_ = 0;
  n_near_ph_hits_ = 0;
  photon_territory_hits_ = 0;
}

void EcalMipTrackingProcessor::produce(framework::Event &event) {
  auto start = std::chrono::high_resolution_clock::now();

  ldmx::EcalMipResult mip_result;
  clearProcessor();

  // Get the Ecal Geometry
  geometry_ = &getCondition<ldmx::EcalGeometry>(
      ldmx::EcalGeometry::CONDITIONS_OBJECT_NAME);

  // Read in hits_ near photon from EcalVetoProcessor
  auto ecal_veto_result = event.getObject<ldmx::EcalVetoResult>(
      ecal_collection_name_, ecal_pass_name_);
  auto ecal_trajectory_info = event.getObject<ldmx::EcalTrajectoryInfo>(
      mip_collection_name_, mip_pass_name_);
  std::vector<XYCoords> ele_trajectory, photon_trajectory,
      ele_trajectory_at_target;
  std::vector<ldmx::HitData> tracking_hit_list;
  ldmx_log(trace) << "EcalMipTrackingProcessor::produce() called";
  ele_trajectory = ecal_trajectory_info.getEleTrajectory();
  photon_trajectory = ecal_trajectory_info.getPhotonTrajectory();
  tracking_hit_list = ecal_trajectory_info.getTrackingHitList();
  n_readout_hits_ = ecal_veto_result.getNReadoutHits();
  nevents_++;
  // Now inputting Lines 753-1178 of the original EcalVetoProcessor
  // ------------------------------------------------------
  // MIP tracking starts here

  /* Goal:  Calculate
   *  n_straight_tracks (self-explanatory),
   *  n_linreg_tracks (tracks found by linreg algorithm),
   */

  // Find epAng and epSep, and prepare EP trajectory vectors:
  ROOT::Math::XYZVector e_traj_start;
  ROOT::Math::XYZVector e_traj_end;
  ROOT::Math::XYZVector p_traj_start;
  ROOT::Math::XYZVector p_traj_end;
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
  } else {
    // Electron trajectory is missing, so place trajectories far outside the
    // ECal to ensure they don't interfere with tracking.
    e_traj_start = ROOT::Math::XYZVector(999, 999, geometry_->getZPosition(0));
    e_traj_end = ROOT::Math::XYZVector(
        999, 999, geometry_->getZPosition((n_ecal_layers_ - 1)));
    p_traj_start =
        ROOT::Math::XYZVector(1000, 1000, geometry_->getZPosition(0));
    p_traj_end = ROOT::Math::XYZVector(
        1000, 1000, geometry_->getZPosition((n_ecal_layers_ - 1)));
  }
  // Near photon step:  Find the first layer_ of the ECal where a hit near the
  // projected photon trajectory is found Currently unusued pending further
  // study; performance has dropped between v9 and v12. Currently used in
  // segmipBDT
  first_near_ph_layer_ = n_ecal_layers_ - 1;

  // If no photon trajectory, leave this at the default (ECal back)
  ldmx_log(trace) << "Finding first near photon layer_";
  if (!photon_trajectory.empty()) {
    for (std::vector<ldmx::HitData>::iterator it = tracking_hit_list.begin();
         it != tracking_hit_list.end(); ++it) {
      float eh_dist =
          sqrt(pow((*it).pos_.X() - photon_trajectory[(*it).layer_].first, 2) +
               pow((*it).pos_.Y() - photon_trajectory[(*it).layer_].second, 2));
      // TODO: this 8.7 should be not hardcoded
      if (eh_dist < 8.7) {
        n_near_ph_hits_++;
        if ((*it).layer_ < first_near_ph_layer_) {
          first_near_ph_layer_ = (*it).layer_;
        }
      }
    }
    ldmx_log(trace) << "First near photon layer_: " << first_near_ph_layer_;
  }

  // Territories limited to tracking_hit_list
  ROOT::Math::XYZVector g_toe = (e_traj_start - p_traj_start).Unit();
  // TODO what is this 8.7 here???
  ROOT::Math::XYZVector origin = p_traj_start + 0.5 * 8.7 * g_toe;
  ldmx_log(trace) << "Origin of photon territory: " << origin.X() << ", "
                  << origin.Y() << ", " << origin.Z();
  if (!ele_trajectory.empty()) {
    for (auto &hit_data : tracking_hit_list) {
      ROOT::Math::XYZVector hit_pos = hit_data.pos_;
      ROOT::Math::XYZVector hit_prime = hit_pos - origin;
      if (hit_prime.Dot(g_toe) <= 0) {
        photon_territory_hits_++;
      }
    }
    ldmx_log(trace) << "Photon territory hits_: " << photon_territory_hits_;
  } else {
    photon_territory_hits_ = n_readout_hits_;
  }

  // ------------------------------------------------------
  // Find straight MIP tracks:

  std::sort(
      tracking_hit_list.begin(), tracking_hit_list.end(),
      [](ldmx::HitData ha, ldmx::HitData hb) { return ha.layer_ > hb.layer_; });
  // For merging tracks:  Need to keep track of existing tracks
  // Candidate tracks to merge in will always be in front of the current track
  // (lower z_), so only store the last hit 3-layer_ vector:  each track = vector
  // of 3-tuples (xy+layer_).
  std::vector<std::vector<ldmx::HitData>> track_list;

  // print tracking_hit_list

  ldmx_log(trace) << "====== Tracking hit list (original) length "
                  << tracking_hit_list.size() << " ======";
  for (int i = 0; i < tracking_hit_list.size(); i++) {
    ldmx_log(trace) << "[" << tracking_hit_list[i].pos_.X() << ", "
                    << tracking_hit_list[i].pos_.Y() << ", "
                    << tracking_hit_list[i].layer_ << "], ";
  }
  ldmx_log(trace) << "====== END OF Tracking hit list ======";

  // in v14 minR is 4.17 mm
  // while maxR is 4.81 mm
  float cell_width = 2 * geometry_->getCellMaxR();
  for (int i_hit = 0; i_hit < tracking_hit_list.size(); i_hit++) {
    // list of hit numbers in track (34 = maximum theoretical length)
    int track[34];
    int current_hit{i_hit};
    int track_len{1};

    track[0] = i_hit;

    // Search for hits_ to add to the track:
    // repeatedly find hits_ in the front two layers with same x_ & y_ positions
    // but since v14 the odd layers are offset, so we allow half a cell_width
    // deviation and then add to track until no more hits_ are found
    int j_hit = i_hit;
    while (j_hit < tracking_hit_list.size()) {
      if ((tracking_hit_list[j_hit].layer_ ==
               tracking_hit_list[current_hit].layer_ - 1 ||
           tracking_hit_list[j_hit].layer_ ==
               tracking_hit_list[current_hit].layer_ - 2) &&
          std::abs(tracking_hit_list[j_hit].pos_.X() -
                   tracking_hit_list[current_hit].pos_.X()) <=
              0.5 * cell_width &&
          std::abs(tracking_hit_list[j_hit].pos_.Y() -
                   tracking_hit_list[current_hit].pos_.Y()) <=
              0.5 * cell_width) {
        track[track_len] = j_hit;
        track_len++;
        current_hit = j_hit;
      }
      j_hit++;
    }

    // Confirm that the track is valid:
    if (track_len < 2) continue;  // Track must contain at least 2 hits_
    float closest_e = ecal::distTwoLines(
        tracking_hit_list[track[0]].pos_,
        tracking_hit_list[track[track_len - 1]].pos_, e_traj_start, e_traj_end);
    float closest_p = ecal::distTwoLines(
        tracking_hit_list[track[0]].pos_,
        tracking_hit_list[track[track_len - 1]].pos_, p_traj_start, p_traj_end);
    // Make sure that the track is near the photon trajectory and away from the
    // electron trajectory Details of these constraints may be revised
    if (closest_p > cell_width and closest_e < 2 * cell_width) continue;
    if (track_len < 4 and closest_e > closest_p) continue;

    ldmx_log(debug) << "====== After rejection for MIP tracking ======";
    ldmx_log(debug) << "current hit: [" << tracking_hit_list[i_hit].pos_.X()
                    << ", " << tracking_hit_list[i_hit].pos_.Y() << ", "
                    << tracking_hit_list[i_hit].layer_ << "]";

    for (int k = 0; k < track_len; k++) {
      ldmx_log(debug) << "track[" << k << "] position = ["
                      << tracking_hit_list[track[k]].pos_.X() << ", "
                      << tracking_hit_list[track[k]].pos_.Y() << ", "
                      << tracking_hit_list[track[k]].layer_ << "]";
    }

    // if track found, increment n_straight_tracks and remove all hits_ in track
    // from future consideration
    if (track_len >= 2) {
      std::vector<ldmx::HitData> temp_track_list;
      int n_remove = 0;
      for (int k_hit = 0; k_hit < track_len; k_hit++) {
        temp_track_list.push_back(tracking_hit_list[track[k_hit] - n_remove]);
        tracking_hit_list.erase(tracking_hit_list.begin() + track[k_hit] -
                                n_remove);
        n_remove++;
      }
      // print tracking_hit_list

      ldmx_log(trace) << "====== Tracking hit list (after erase) length "
                      << tracking_hit_list.size() << " ======";
      for (int i = 0; i < tracking_hit_list.size(); i++) {
        ldmx_log(trace) << "[" << tracking_hit_list[i].pos_.X() << ", "
                        << tracking_hit_list[i].pos_.Y() << ", "
                        << tracking_hit_list[i].layer_ << "] ";
      }
      ldmx_log(trace) << "====== END OF Tracking hit list ======";

      track_list.push_back(temp_track_list);
      // The *current* hit will have been removed, so i_hit is currently
      // pointing to the next hit. Decrement i_hit so no hits_ will get skipped
      // by i_hit++
      i_hit--;
    }
  }

  ldmx_log(debug) << "Straight tracks found (before merge): "
                  << track_list.size();

  for (int i_track = 0; i_track < track_list.size(); i_track++) {
    ldmx_log(trace) << "Track " << i_track << ":";
    for (int i_hit = 0; i_hit < track_list[i_track].size(); i_hit++) {
      ldmx_log(trace) << "  Hit " << i_hit << ": ["
                      << track_list[i_track][i_hit].pos_.X() << ", "
                      << track_list[i_track][i_hit].pos_.Y() << ", "
                      << track_list[i_track][i_hit].layer_ << "]" << std::endl;
    }
  }

  // Optional addition:  Merge nearby straight tracks.  Not necessary for veto.
  // Criteria:  consider tail of track.  Merge if head of next track is 1/2
  // layers behind, within 1 cell of xy position.
  ldmx_log(debug) << "Beginning track merging using " << track_list.size()
                  << " tracks";

  for (int track_i = 0; track_i < track_list.size(); track_i++) {
    // for each track, check the remainder of the track list for compatible
    // tracks
    std::vector<ldmx::HitData> base_track = track_list[track_i];
    ldmx::HitData tail_hitdata =
        base_track.back();  // xylayer of last hit in track
    ldmx_log(trace) << "  Considering track " << track_i;
    for (int track_j = track_i + 1; track_j < track_list.size(); track_j++) {
      std::vector<ldmx::HitData> checking_track = track_list[track_j];
      ldmx::HitData head_hitdata = checking_track.front();
      // if 1-2 layers behind, and xy within one cell...
      if ((head_hitdata.layer_ == tail_hitdata.layer_ + 1 ||
           head_hitdata.layer_ == tail_hitdata.layer_ + 2) &&
          pow(pow(head_hitdata.pos_.X() - tail_hitdata.pos_.X(), 2) +
                  pow(head_hitdata.pos_.Y() - tail_hitdata.pos_.Y(), 2),
              0.5) <= cell_width) {
        // ...then append the second track to the first one and delete it
        // NOTE:  TO ADD:  (tracking_hit_list[i_hit].pos_ -
        // tracking_hit_list[j_hit].pos_).R()
        ldmx_log(trace) << "     ** Compatible track found at index_ "
                        << track_j;
        ldmx_log(trace) << "     Tail xylayer: " << head_hitdata.pos_.X() << ","
                        << head_hitdata.pos_.Y() << "," << head_hitdata.layer_;
        ldmx_log(trace) << "     Head xylayer: " << tail_hitdata.pos_.X() << ","
                        << tail_hitdata.pos_.Y() << "," << tail_hitdata.layer_;
        for (int hit_k = 0; hit_k < checking_track.size(); hit_k++) {
          base_track.push_back(track_list[track_j][hit_k]);
        }
        track_list[track_i] = base_track;
        track_list.erase(track_list.begin() + track_j);
        break;
      }
    }
  }
  n_straight_tracks_ = track_list.size();
  // print the track list
  ldmx_log(debug) << "Straight tracks found (after merge): "
                  << n_straight_tracks_;
  for (int track_i = 0; track_i < track_list.size(); track_i++) {
    ldmx_log(debug) << "Track " << track_i << ":";
    for (int hit_i = 0; hit_i < track_list[track_i].size(); hit_i++) {
      ldmx_log(debug) << "  Hit " << hit_i << ": ["
                      << track_list[track_i][hit_i].pos_.X() << ", "
                      << track_list[track_i][hit_i].pos_.Y() << ", "
                      << track_list[track_i][hit_i].layer_ << "]";
    }
  }

  auto straight_tracks = std::chrono::high_resolution_clock::now();
  profiling_map_["straight_tracks"] +=
      std::chrono::duration<double, std::milli>(straight_tracks - start)
          .count();
  // ------------------------------------------------------
  // Linreg tracking:
  ldmx_log(info) << "Finding linreg tracks from a total of "
                 << tracking_hit_list.size() << " hits_ using a radius_ of "
                 << linreg_radius_ << " mm";

  for (int i_hit = 0; i_hit < 0; i_hit++) {
    // for (int i_hit = 0; i_hit < tracking_hit_list.size(); i_hit++) {
    //  Hits being considered at a given time
    std::vector<int> hits_in_region;
    TMatrixD vm(3, 3);
    TMatrixD hdt(3, 3);
    ROOT::Math::XYZVector slope_vec;
    ROOT::Math::XYZVector h_mean;
    ROOT::Math::XYZVector h_point;
    float r_corr_best{0.0};
    // Temp array having 3 potential hits_
    int hit_nums[3];
    // From the above which are passing the correlation reqs
    int best_hit_nums[3];

    hits_in_region.push_back(i_hit);
    // Find all hits_ within 2 cells of the primary hit:
    for (int j_hit = 0; j_hit < tracking_hit_list.size(); j_hit++) {
      // Dont try to put hits_ on the same layer_ to the lin-reg track
      if (tracking_hit_list[i_hit].pos_.Z() ==
          tracking_hit_list[j_hit].pos_.Z()) {
        continue;
      }
      float dist_to_hit =
          (tracking_hit_list[i_hit].pos_ - tracking_hit_list[j_hit].pos_).R();
      // This distance optimized to give the best significance
      // it used to be 2*cell_width, i.e. 4.81 mm
      // note, the layers in the back have a separation of 22.3
      if (dist_to_hit <= 2 * linreg_radius_) {
        hits_in_region.push_back(j_hit);
      }
    }
    // Found a track that passed the lin-reg reqs
    bool best_lin_reg_found{false};

    ldmx_log(debug) << "There are " << hits_in_region.size()
                    << " hits_ within a radius_ of " << linreg_radius_ << " mm";
    // Look at combinations of hits_ within the region (do not consider the same
    // combination twice):
    hit_nums[0] = i_hit;
    for (int j_hit_in_reg = 1; j_hit_in_reg < hits_in_region.size() - 1;
         j_hit_in_reg++) {
      // We require (exactly) 3 hits_ for the lin-reg track building
      if (hits_in_region.size() < 3) break;
      hit_nums[1] = hits_in_region[j_hit_in_reg];
      for (int k_hit_reg = j_hit_in_reg + 1; k_hit_reg < hits_in_region.size();
           k_hit_reg++) {
        hit_nums[2] = hits_in_region[k_hit_reg];
        const auto &p0 = tracking_hit_list[hit_nums[0]].pos_;
        const auto &p1 = tracking_hit_list[hit_nums[1]].pos_;
        const auto &p2 = tracking_hit_list[hit_nums[2]].pos_;

        h_mean = (p0 + p1 + p2) / 3.0;

        double p_arr[3][3] = {{p0.X(), p0.Y(), p0.Z()},
                              {p1.X(), p1.Y(), p1.Z()},
                              {p2.X(), p2.Y(), p2.Z()}};

        // Compute mean in an indexable array
        double mean_arr[3] = {(p0.X() + p1.X() + p2.X()) / 3.0,
                              (p0.Y() + p1.Y() + p2.Y()) / 3.0,
                              (p0.Z() + p1.Z() + p2.Z()) / 3.0};

        for (int h_ind = 0; h_ind < 3; ++h_ind) {
          for (int l_ind = 0; l_ind < 3; ++l_ind) {
            hdt(h_ind, l_ind) = p_arr[h_ind][l_ind] - mean_arr[l_ind];
          }
        }

        // Perform "linreg" on selected points
        // Calculate the determinant of the matrix
        double determinant =
            hdt(0, 0) * (hdt(1, 1) * hdt(2, 2) - hdt(1, 2) * hdt(2, 1)) -
            hdt(0, 1) * (hdt(1, 0) * hdt(2, 2) - hdt(1, 2) * hdt(2, 0)) +
            hdt(0, 2) * (hdt(1, 0) * hdt(2, 1) - hdt(1, 1) * hdt(2, 0));
        // Exit early if the matrix is singular (i.e. det = 0)
        if (determinant == 0) continue;
        // Perform matrix decomposition with SVD
        TDecompSVD svd_obj(hdt);
        bool decomposed = svd_obj.Decompose();
        if (!decomposed) continue;

        // First col of V matrix is the slope of the best-fit line
        vm = svd_obj.GetV();
        slope_vec.SetX(vm[0][0]);
        slope_vec.SetY(vm[0][1]);
        slope_vec.SetZ(vm[0][2]);
        // h_mean, h_point are points on the best-fit line
        h_point = slope_vec + h_mean;
        // linreg complete:  Now have best-fit line for 3 hits_ under
        // consideration Check whether the track is valid:  r^2 must be high,
        // and the track must plausibly originate from the photon
        float closest_e =
            ecal::distTwoLines(h_mean, h_point, e_traj_start, e_traj_end);
        float closest_p =
            ecal::distTwoLines(h_mean, h_point, p_traj_start, p_traj_end);
        // Projected track must be close to the photon; details may change after
        // future study.
        if (closest_p > cell_width or closest_e < 1.5 * cell_width) continue;
        // find r^2
        // ~variance
        float vrnc = (tracking_hit_list[hit_nums[0]].pos_ - h_mean).R() +
                     (tracking_hit_list[hit_nums[1]].pos_ - h_mean).R() +
                     (tracking_hit_list[hit_nums[2]].pos_ - h_mean).R();
        // sum of |errors|
        float sumerr = ecal::distPtToLine(tracking_hit_list[hit_nums[0]].pos_,
                                          h_mean, h_point) +
                       ecal::distPtToLine(tracking_hit_list[hit_nums[1]].pos_,
                                          h_mean, h_point) +
                       ecal::distPtToLine(tracking_hit_list[hit_nums[2]].pos_,
                                          h_mean, h_point);
        float r_corr = 1 - sumerr / vrnc;
        // Check whether r^2 exceeds a low minimum r_corr:  "Fake" tracks are
        // still much more common in background, so making the algorithm
        // oversensitive doesn't lower performance significantly
        if (r_corr > r_corr_best and r_corr > .6) {
          r_corr_best = r_corr;
          // Only looking for 3-hit tracks currently
          best_lin_reg_found = true;
          for (int k = 0; k < 3; k++) {
            best_hit_nums[k] = hit_nums[k];
          }
        }
      }  // end loop on hits_ in the region
    }  // end 2nd loop on hits_ in the region

    // Continue early if not hits_ on track
    if (!best_lin_reg_found) continue;
    // Otherwise increase the number of lin-reg tracks
    n_linreg_tracks_++;
    ldmx_log(debug) << " Lin-reg track " << n_linreg_tracks_;
    for (int final_hit_index = 0; final_hit_index < 3; final_hit_index++) {
      ldmx_log(debug) << "   Hit " << final_hit_index << " ["
                      << tracking_hit_list[best_hit_nums[final_hit_index]].pos_.X()
                      << ", "
                      << tracking_hit_list[best_hit_nums[final_hit_index]].pos_.Y()
                      << ", "
                      << tracking_hit_list[best_hit_nums[final_hit_index]].pos_.Z()
                      << "] ";
    }

    // Exclude all hits_ in a found track from further consideration:
    for (int l_hit = 0; l_hit < 3; l_hit++) {
      tracking_hit_list.erase(tracking_hit_list.begin() + best_hit_nums[l_hit]);
    }
    i_hit--;
  }  // end loop on all hits_
  ldmx_log(info) << " MIP tracking completed; found " << n_straight_tracks_
                 << " straight tracks and " << n_linreg_tracks_
                 << " lin-reg tracks";

  auto linreg_tracks = std::chrono::high_resolution_clock::now();
  profiling_map_["linreg_tracks"] +=
      std::chrono::duration<double, std::milli>(linreg_tracks - straight_tracks)
          .count();

  mip_result.setVariables(n_straight_tracks_, n_linreg_tracks_,
                          first_near_ph_layer_, n_near_ph_hits_,
                          photon_territory_hits_);

  event.add(mip_result_name_, mip_result);

  auto end = std::chrono::high_resolution_clock::now();
  auto time_diff = end - start;
  processing_time_ +=
      std::chrono::duration<double, std::milli>(time_diff).count();
}

void EcalMipTrackingProcessor::onProcessEnd() {
  ldmx_log(info) << "AVG Time/Event: " << std::fixed << std::setprecision(2)
                 << processing_time_ / nevents_ << " ms";

  ldmx_log(info) << "Breakdown::";

  ldmx_log(info) << "straight_tracks        Avg Time/Event = " << std::fixed
                 << std::setprecision(3)
                 << profiling_map_["straight_tracks"] / nevents_ << " ms";

  ldmx_log(info) << "linreg_tracks        Avg Time/Event = " << std::fixed
                 << std::setprecision(3)
                 << profiling_map_["linreg_tracks"] / nevents_ << " ms";
}

}  // namespace ecal

DECLARE_PRODUCER(ecal::EcalMipTrackingProcessor);
#include "Tracking/Reco/LinearSeedFinder.h"

#include "Eigen/Dense"

namespace tracking {
namespace reco {

LinearSeedFinder::LinearSeedFinder(const std::string& name,
                                   framework::Process& process)
    : TrackingGeometryUser(name, process) {}

void LinearSeedFinder::onProcessStart() {
  truth_matching_tool_ = std::make_shared<tracking::sim::TruthMatchingTool>();
}

void LinearSeedFinder::configure(framework::config::Parameters& parameters) {
  // Output seed name
  out_seed_collection_ = parameters.getParameter<std::string>(
      "out_seed_collection", getName() + "LinearRecoilSeedTracks");

  // Input strip hits_
  input_hits_collection_ = parameters.getParameter<std::string>(
      "input_hits_collection", "DigiRecoilSimHits");
  input_rec_hits_collection_ = parameters.getParameter<std::string>(
      "input_rec_hits_collection", "EcalRecHits");

  input_pass_name_ =
      parameters.getParameter<std::string>("input_pass_name", "");

  sim_particles_passname_ =
      parameters.getParameter<std::string>("sim_particles_passname");

  sim_particles_events_passname_ =
      parameters.getParameter<std::string>("sim_particles_events_passname");

  // the uncertainty is sigma_x = 6 microns and sigma_y = 20./sqrt(12)
  recoil_uncertainty_ = parameters.getParameter<std::vector<double>>(
      "recoil_uncertainty", {0.006, 0.085});
  ecal_uncertainty_ =
      parameters.getParameter<double>("ecal_uncertainty", {3.87});
  ecal_distance_threshold_ =
      parameters.getParameter<double>("ecal_distance_threshold");
  ecal_first_layer_z_threshold_ =
      parameters.getParameter<double>("ecal_first_layer_z_threshold");

  layer12_midpoint_ = parameters.getParameter<double>("layer12_midpoint");
  layer23_midpoint_ = parameters.getParameter<double>("layer23_midpoint");
  layer34_midpoint_ = parameters.getParameter<double>("layer34_midpoint");
}

void LinearSeedFinder::produce(framework::Event& event) {
  auto start = std::chrono::high_resolution_clock::now();
  std::vector<ldmx::StraightTrack> straight_seed_tracks;
  n_events_++;
  auto tg{geometry()};

  const std::vector<ldmx::Measurement> recoil_hits =
      event.getCollection<ldmx::Measurement>(input_hits_collection_,
                                             input_pass_name_);
  const std::vector<ldmx::EcalHit> ecal_rec_hit =
      event.getCollection<ldmx::EcalHit>(input_rec_hits_collection_,
                                         input_pass_name_);

  std::vector<std::array<double, 3>> first_layer_ecal_rec_hits;

  // Find RecHits at first layer_ of ECal
  for (const auto& x_ecal : ecal_rec_hit) {
    if (x_ecal.getZPos() < ecal_first_layer_z_threshold_) {
      first_layer_ecal_rec_hits.push_back(
          {x_ecal.getZPos(), x_ecal.getXPos(), x_ecal.getYPos()});
    }  // if first layer_ of Ecal
  }  // for positions in ecalRecHit

  // Check if we would fit empty seeds, if so: end tracking
  if ((recoil_hits.size() < 2) || (first_layer_ecal_rec_hits.empty()) ||
      (uniqueLayersHit(recoil_hits) < 2)) {
    n_missing_++;
    n_seeds_ += straight_seed_tracks.size();
    event.add(out_seed_collection_, straight_seed_tracks);
    return;
  }

  // Setup truth map
  std::map<int, ldmx::SimParticle> particle_map;
  if (event.exists("SimParticles", sim_particles_events_passname_)) {
    particle_map = event.getMap<int, ldmx::SimParticle>(
        "SimParticles", sim_particles_passname_);
    truth_matching_tool_->setup(particle_map, recoil_hits);
  }

  std::vector<
      std::tuple<ldmx::Measurement, ldmx::SimTrackerHit, ldmx::SimTrackerHit>>
      first_two_layers;
  std::vector<
      std::tuple<ldmx::Measurement, ldmx::SimTrackerHit, ldmx::SimTrackerHit>>
      second_two_layers;

  const std::vector<ldmx::SimTrackerHit> recoil_sim_hits =
      event.getCollection<ldmx::SimTrackerHit>("RecoilSimHits",
                                               input_pass_name_);
  const std::vector<ldmx::SimTrackerHit> scoring_hits =
      event.getCollection<ldmx::SimTrackerHit>("TargetScoringPlaneHits",
                                               input_pass_name_);

  // Index all sim hits_ by track ID
  std::unordered_map<int, std::vector<const ldmx::SimTrackerHit*>>
      sim_hits_by_track_id;
  for (const auto& hit : recoil_sim_hits) {
    sim_hits_by_track_id[hit.getTrackID()].push_back(&hit);
  }  // for sim hits_

  // Index scoring hits_ by track ID (one positive scoring plane hit per trackID)
  std::unordered_map<int, const ldmx::SimTrackerHit*> scoring_hit_map;
  for (const auto& sp_hit : scoring_hits) {
    if (sp_hit.getPosition()[2] > 0)
      scoring_hit_map[sp_hit.getTrackID()] = &sp_hit;
  }  // for sp hits_

  for (const auto& point : recoil_hits) {
    // x_ is in tracking coordinates, z_ is in ldmx coordinates
    float x_ = point.getGlobalPosition()[0];
    int track_id = point.getTrackIds()[0];

    // get the key value = track_id
    auto sim_range_it = sim_hits_by_track_id.find(track_id);
    if (sim_range_it == sim_hits_by_track_id.end()) continue;

    // access map value at track_id
    const auto& sim_hits = sim_range_it->second;

    for (const auto* sim_hit : sim_hits) {
      float z_ = sim_hit->getPosition()[2];

      if (x_ < layer12_midpoint_) {
        if (z_ < layer12_midpoint_) {
          auto sp_it = scoring_hit_map.find(track_id);
          if (sp_it != scoring_hit_map.end()) {
            first_two_layers.emplace_back(point, *sim_hit, *sp_it->second);
            break;
          }  // add the associated scoring plane hit (will be needed for 3D
             // reconstruction)
        }  // associate 1st layer_ sim hit
      }  // check if recoil hit is 1st layer_
      else if (x_ < layer23_midpoint_) {
        if (z_ > layer12_midpoint_ && z_ < layer23_midpoint_) {
          first_two_layers.emplace_back(point, *sim_hit, ldmx::SimTrackerHit());
          break;
        }  // associate 2nd layer_ sim hit
      }  // check if recoil hit is 2nd layer_
      else if (x_ < layer34_midpoint_) {
        if (z_ > layer23_midpoint_ && z_ < layer34_midpoint_) {
          auto sp_it = scoring_hit_map.find(track_id);
          if (sp_it != scoring_hit_map.end()) {
            second_two_layers.emplace_back(point, *sim_hit, *sp_it->second);
            break;
          }  // add the associated scoring plane hit (will be needed for 3D
             // reconstruction)
        }  // associate 3rd layer_ sim hits_
      }  // check if recoil hit is 3rd layer_
      else {
        if (z_ > layer34_midpoint_) {
          second_two_layers.emplace_back(point, *sim_hit,
                                         ldmx::SimTrackerHit());
          break;
        }  // associate 4th layer_ sim hits_
      }  // check if recoil hits_ is 4th layer_

    }  // loop through simhits
  }  // loop through recoil hits_

  // Reconstruct 3D sensor points on which to do fitting
  auto first_sensor_combos = processMeasurements(first_two_layers, tg);
  auto second_sensor_combos = processMeasurements(second_two_layers, tg);

  for (const auto& [first_combo_3d_point, first_layer_one, first_layer_two] :
       first_sensor_combos) {
    std::tuple<std::array<double, 3>, ldmx::Measurement,
               std::optional<ldmx::Measurement>>
        first_sensor_point;

    if (first_layer_two.has_value()) {
      first_sensor_point = {first_combo_3d_point,
                            std::get<ldmx::Measurement>(first_layer_one),
                            std::get<ldmx::Measurement>(*first_layer_two)};
    }  // check whether we did reconstruction or...
    else {
      first_sensor_point = {first_combo_3d_point,
                            std::get<ldmx::Measurement>(first_layer_one),
                            std::nullopt};
    }  //...we are taking only one layer_ as the measurement (axial or stereo)

    for (const auto& [second_combo_3d_point, second_layer_one,
                      second_layer_two] : second_sensor_combos) {
      std::tuple<std::array<double, 3>, ldmx::Measurement,
                 std::optional<ldmx::Measurement>>
          second_sensor_point;
      if (second_layer_two.has_value()) {
        second_sensor_point = {second_combo_3d_point,
                               std::get<ldmx::Measurement>(second_layer_one),
                               std::get<ldmx::Measurement>(*second_layer_two)};
      }  // check whether we did reconstruction or...
      else {
        second_sensor_point = {second_combo_3d_point,
                               std::get<ldmx::Measurement>(second_layer_one),
                               std::nullopt};
      }  //...we are taking only one layer_ as the measurement (axial or stereo)

      for (const auto& rec_hit : first_layer_ecal_rec_hits) {
        // Do fitting on 2 sensor + 1 recHit combinations = 1 degree of freedom
        // for linear fit
        ldmx::StraightTrack seed_track =
            SeedTracker(first_sensor_point, second_sensor_point, rec_hit);

        // Seed passed RecHit distance check, add it
        if (seed_track.getChi2() > 0.0) {
          straight_seed_tracks.push_back(seed_track);
        }  // if chi2 > 0
      }  // for rec_hits
    }  // for second recoil tracker
  }  // for first recoil tracker

  n_seeds_ += straight_seed_tracks.size();
  event.add(out_seed_collection_, straight_seed_tracks);

  auto end = std::chrono::high_resolution_clock::now();

  auto diff = end - start;
  processing_time_ += std::chrono::duration<double, std::milli>(diff).count();

  first_layer_ecal_rec_hits.clear();
  straight_seed_tracks.clear();

}  // produce

ldmx::StraightTrack LinearSeedFinder::SeedTracker(
    const std::tuple<std::array<double, 3>, ldmx::Measurement,
                     std::optional<ldmx::Measurement>>
        recoil_one,
    const std::tuple<std::array<double, 3>, ldmx::Measurement,
                     std::optional<ldmx::Measurement>>
        recoil_two,
    const std::array<double, 3> ecal_one) {
  auto [sensor1, layer1, layer2] = recoil_one;
  auto [sensor2, layer3, layer4] = recoil_two;
  std::vector<ldmx::Measurement> all_points;

  // TODO: in the case where we don't have all 4 hits_, we will be fitting a
  // sensor (weighted average of two layers) + single layer_
  // TODO: or fitting two single layers. Currently, the single layer_ point has
  // the uncertainty of a sensor assigned to it,
  // TODO: but this is not a realistic uncertainty for a single layer_...
  // IF all layers are well-defined, this sequence will add layer1, 2, 3, 4 to
  // the allPoints vector
  all_points.push_back(layer1);

  // if layer2 doesn't exist (has_value == False), then the layer1 we added
  // is either layer1 or 2, depending on which one has_value
  if (layer2.has_value()) {
    all_points.push_back(*layer2);
  }

  all_points.push_back(layer3);

  // if layer4 doesn't exist (has_value == False), then the layer3 we added
  // is either layer3 or 4, depending on which one has_value
  if (layer4.has_value()) {
    all_points.push_back(*layer4);
  }

  // Fit the 3 points to a 3D straight line, find track location at first layer_
  // of Ecal, check distance to recHit used in fitting
  // m = slope ; b = intercept
  auto [m_x, b_x, m_y, b_y, seed_cov] = fit3DLine(sensor1, sensor2, ecal_one);
  std::array<double, 3> temp_extrapolated_point = {
      ecal_one[0], m_x * ecal_one[0] + b_x, m_y * ecal_one[0] + b_y};
  double temp_distance = calculateDistance(temp_extrapolated_point, ecal_one);

  ldmx::StraightTrack trk = ldmx::StraightTrack();

  if (temp_distance < ecal_distance_threshold_) {
    trk.setSlopeX(m_x);
    trk.setInterceptX(b_x);
    trk.setSlopeY(m_y);
    trk.setInterceptY(b_y);
    trk.setTheta(std::atan2(m_y, std::sqrt(1 + m_x * m_x)));
    trk.setPhi(std::atan2(m_x, 1.0));

    trk.setAllSensorPoints(all_points);
    trk.setFirstSensorPosition(sensor1);
    trk.setSecondSensorPosition(sensor2);
    trk.setFirstLayerEcalRecHit(ecal_one);
    trk.setDistancetoRecHit(temp_distance);

    trk.setTargetLocation(0.0, b_x, b_y);
    trk.setEcalLayer1Location(temp_extrapolated_point);
    trk.setChi2(
        globalChiSquare(sensor1, sensor2, ecal_one, m_x, m_y, b_x, b_y));
    trk.setNhits(3);
    trk.setNdf(1);

    trk.setCov(seed_cov);

    // truth matching
    if (truth_matching_tool_->configured()) {
      auto truth_info = truth_matching_tool_->TruthMatch(all_points);
      trk.setTrackID(truth_info.trackID);
      trk.setPdgID(truth_info.pdgID);
      trk.setTruthProb(truth_info.truthProb);
    }

    return trk;

  }  // if (track is close enough to EcalRecHit)
  else {
    trk.setChi2(-1);
    return trk;
  }  // else (does not pass the threshold)
}  // SeedTracker

void LinearSeedFinder::onProcessEnd() {
  ldmx_log(info) << "AVG Time/Event: " << std::fixed << std::setprecision(1)
                 << processing_time_ / n_events_ << " ms";
  ldmx_log(info) << "Total Seeds/Events: " << n_seeds_ << "/" << n_events_;
  ldmx_log(info) << "not enough seed points " << n_missing_;

}  // onProcessEnd

std::array<double, 3> LinearSeedFinder::getPointAtZ(
    std::array<double, 3> target, std::array<double, 3> measurement,
    double z_target) {
  double slope_x = (measurement[1] - target[0]) / (measurement[0] - target[2]);
  double slope_y = (measurement[2] - target[1]) / (measurement[0] - target[2]);

  double intercept_x = target[0] - slope_x * target[2];
  double intercept_y = target[1] - slope_y * target[2];

  double x_target = slope_x * z_target + intercept_x;
  double y_target = slope_y * z_target + intercept_y;

  return {z_target, x_target, y_target};
}

std::vector<std::tuple<
    std::array<double, 3>,
    std::tuple<ldmx::Measurement, ldmx::SimTrackerHit, ldmx::SimTrackerHit>,
    std::optional<std::tuple<ldmx::Measurement, ldmx::SimTrackerHit,
                             ldmx::SimTrackerHit>>>>
LinearSeedFinder::processMeasurements(
    const std::vector<std::tuple<ldmx::Measurement, ldmx::SimTrackerHit,
                                 ldmx::SimTrackerHit>>& measurements,
    const geo::TrackersTrackingGeometry& tg) {
  std::vector<
      std::tuple<ldmx::Measurement, ldmx::SimTrackerHit, ldmx::SimTrackerHit>>
      axial_measurements;
  std::vector<
      std::tuple<ldmx::Measurement, ldmx::SimTrackerHit, ldmx::SimTrackerHit>>
      stereo_measurements;
  std::vector<std::tuple<
      std::array<double, 3>,
      std::tuple<ldmx::Measurement, ldmx::SimTrackerHit, ldmx::SimTrackerHit>,
      std::optional<std::tuple<ldmx::Measurement, ldmx::SimTrackerHit,
                               ldmx::SimTrackerHit>>>>
      points_with_measurement;
  Acts::Vector3 dummy{0., 0., 0.};

  // Separate measurements into axial and stereo based on layerID
  for (const auto& [measurement, sim_hit, scoring_hit] : measurements) {
    if (measurement.getLayerID() % 2 == 0) {
      axial_measurements.emplace_back(measurement, sim_hit, scoring_hit);
    } else {
      stereo_measurements.emplace_back(measurement, sim_hit, scoring_hit);
    }
  }

  // If there are measurements in both axial and stereo layer_:
  // Iterate over axial and stereo measurements to compute 3D points
  if (!axial_measurements.empty() && !stereo_measurements.empty()) {
    for (const auto& axial : axial_measurements) {
      const auto& [axial_meas, axial_hit, axial_sp] = axial;

      for (const auto& stereo : stereo_measurements) {
        const auto& [stereo_meas, stereo_hit, stereo_sp] = stereo;

        const Acts::Surface* axial_surface =
            tg.getSurface(axial_meas.getLayerID());
        const Acts::Surface* stereo_surface =
            tg.getSurface(stereo_meas.getLayerID());

        if (!axial_surface || !stereo_surface) continue;

        std::vector<ldmx::SimTrackerHit> sim_hits = {axial_hit, stereo_hit};
        Acts::Vector3 space_point =
            simple3DHitV2(axial_meas, *axial_surface, stereo_meas,
                          *stereo_surface, axial_sp, sim_hits);

        points_with_measurement.push_back(
            {convertToLdmxStdArray(space_point), axial, stereo});
      }
    }
  } else if (!axial_measurements.empty()) {
    // if there are only axial measurements, take them to be our sensor points
    for (const auto& axial : axial_measurements) {
      const auto& [axial_meas, axial_hit, axial_sp] = axial;
      const Acts::Surface* axial_surface =
          tg.getSurface(axial_meas.getLayerID());

      Acts::Vector3 axial_meas_hit = axial_surface->localToGlobal(
          geometry_context(),
          Acts::Vector2(axial_meas.getLocalPosition()[0], 0.0), dummy);

      points_with_measurement.push_back(
          {convertToLdmxStdArray(axial_meas_hit), axial, std::nullopt});
    }
  } else if (!stereo_measurements.empty()) {
    for (const auto& stereo : stereo_measurements) {
      const auto& [stereo_meas, stereo_hit, stereo_sp] = stereo;
      const Acts::Surface* stereo_surface =
          tg.getSurface(stereo_meas.getLayerID());

      Acts::Vector3 stereo_meas_hit = stereo_surface->localToGlobal(
          geometry_context(),
          Acts::Vector2(stereo_meas.getLocalPosition()[0], 0.0), dummy);

      points_with_measurement.push_back(
          {convertToLdmxStdArray(stereo_meas_hit), stereo, std::nullopt});
    }
  }
  return points_with_measurement;
}

// ACTS saves its arrays like (x_, y_, z_)
std::array<double, 3> LinearSeedFinder::convertToLdmxStdArray(
    const Acts::Vector3& vec) {
  return {vec.x(), vec.y(), vec.z()};
}

// Helper function to calculate unit vector by taking advantage of the
// localToGlobal transformation
std::tuple<Acts::Vector3, Acts::Vector3, Acts::Vector3>
LinearSeedFinder::getSurfaceVectors(const Acts::Surface& surface) {
  Acts::Vector3 dummy{0., 0., 0.};
  Acts::Vector3 u =
      surface.localToGlobal(geometry_context(), Acts::Vector2(1, 0), dummy) -
      surface.center(geometry_context());
  Acts::Vector3 v =
      surface.localToGlobal(geometry_context(), Acts::Vector2(0, 1), dummy) -
      surface.center(geometry_context());
  Acts::Vector3 w = u.cross(v).normalized();
  return {u.normalized(), v.normalized(), w};
}

//  estimate the 3d position of the particle as it passes through a stereo/axial
//  pair currently this uses the sim hits_ from the target and the axial sensor
//  to calculate the angle of the particle, which is needed to project the two
//  sensors to the same z_ For real data, we could use the position at the target
//  for the tagger and the axial u position of the measured hit (we only project
//  in x_, which, in MC, is identically x_) assumptions:   axial u-direction is
//  identically global-x_ (tracking-global y_)
//                 sensors' w-directions are aligned with beam (global-z_,
//                 tracking-global x_)
Acts::Vector3 LinearSeedFinder::simple3DHitV2(
    const ldmx::Measurement& axial, const Acts::Surface& axial_surface,
    const ldmx::Measurement& stereo, const Acts::Surface& stereo_surface,
    const ldmx::SimTrackerHit& target_sp,
    std::vector<ldmx::SimTrackerHit> pair_sim_hits) {
  Acts::Vector3 dummy{0., 0., 0.};
  Acts::Vector3 hitOnTarget{target_sp.getPosition()[0],
                            target_sp.getPosition()[1],
                            target_sp.getPosition()[2]};  // x_,y_,z_

  Acts::Vector3 axial_true_global{pair_sim_hits[0].getPosition()[0],
                                  pair_sim_hits[0].getPosition()[1],
                                  pair_sim_hits[0].getPosition()[2]};
  // stereo_true_global is unused
  Acts::Vector3 stereo_true_global{pair_sim_hits[1].getPosition()[0],
                                   pair_sim_hits[1].getPosition()[1],
                                   pair_sim_hits[1].getPosition()[2]};

  Acts::Vector3 simpart_path = axial_true_global - hitOnTarget;
  Acts::Vector3 simpart_unit = simpart_path.normalized();

  // Get global positions for strip origins  ....  actually these are in
  // tracking coordinates!
  Acts::Vector3 axial_origin = axial_surface.center(geometry_context());
  Acts::Vector3 stereo_origin = stereo_surface.center(geometry_context());

  // the tracking-global vector difference between stereo and axial sensor
  // centers
  Acts::Vector3 deltaSensors = stereo_origin - axial_origin;

  // calculate the displacement in tracking global x_ (need to generalize) by
  // going from tracking x_=axial to x_=stereo
  double dx_proj =
      (simpart_unit[0] / simpart_unit[2]) *
      deltaSensors[0];  // this looks weird because simpart_unit is in
                        // global-global and delta sensors is in tracking-global

  // Compute unit vectors for both hits_
  auto [axial_u, axial_v, axial_w] = getSurfaceVectors(axial_surface);
  auto [stereo_u, stereo_v, stereo_w] = getSurfaceVectors(stereo_surface);
  double salpha = dotProduct(axial_v, stereo_u);
  double cosalpha = dotProduct(axial_u, stereo_u);

  // Get sensor local measured coordinates
  // Get local position components
  auto [axial_u_value, axial_v_value] = axial.getLocalPosition();
  auto [stereo_u_value, stereo_v_value] = stereo.getLocalPosition();

  // Manual correction, since v should always be 0 (insensitive direction)
  axial_v_value = 0.0;
  stereo_v_value = 0.0;

  // use the dx_proj as the displacement in u of the axial measurement
  // it's axial_u_value - dx_proj because u is in the -x_ direction
  //  this calculation is in the axial frame
  double v_intercept_useproj =
      (stereo_u_value - (axial_u_value - dx_proj) * cosalpha) / salpha;
  double u_intercept_useproj = axial_u_value - dx_proj;

  // convert to tracking global
  Acts::Vector3 axst_global_useproj = axial_surface.localToGlobal(
      geometry_context(),
      Acts::Vector2(u_intercept_useproj, v_intercept_useproj), dummy);
  Acts::Vector3 dummy_stereo_proj = stereo_surface.localToGlobal(
      geometry_context(),
      Acts::Vector2(u_intercept_useproj, v_intercept_useproj), dummy);

  // we want the reconstructed hit to be at the z_ of the stereo layer_
  Acts::Vector3 reconstructed_hit{dummy_stereo_proj[0], axst_global_useproj[1],
                                  axst_global_useproj[2]};

  ldmx_log(debug) << "The particle projected axst measured position is "
                     "(compare with stereo sim position): "
                  << reconstructed_hit[0] << ", " << reconstructed_hit[1]
                  << ", " << reconstructed_hit[2] << "\n";

  return reconstructed_hit;
}

double LinearSeedFinder::dotProduct(const Acts::Vector3& v1,
                                    const Acts::Vector3& v2) {
  return v1.dot(v2);
}

std::tuple<double, double, double, double, std::vector<double>>
LinearSeedFinder::fit3DLine(const std::array<double, 3>& first_recoil,
                            const std::array<double, 3>& second_recoil,
                            const std::array<double, 3>& ecal) {
  double z_pos1 = first_recoil[0], x_pos1 = first_recoil[1],
         y_pos1 = first_recoil[2];
  double z_pos2 = second_recoil[0], x_pos2 = second_recoil[1],
         y_pos2 = second_recoil[2];
  double z_pos3 = ecal[0], x_pos3 = ecal[1], y_pos3 = ecal[2];

  std::array<double, 6> weights = {
      1 / pow(recoil_uncertainty_[0], 2), 1 / pow(recoil_uncertainty_[1], 2),
      1 / pow(recoil_uncertainty_[0], 2), 1 / pow(recoil_uncertainty_[1], 2),
      1 / pow(ecal_uncertainty_, 2),      1 / pow(ecal_uncertainty_, 2)};

  Eigen::Matrix<double, 6, 4> A_mat;
  Eigen::Matrix<double, 6, 1> d_vec, w_vec;

  // Fill the A matrix (z_, 1, 0, 0) for x_ and (0, 0, z_, 1) for y_
  A_mat << z_pos1, 1, 0, 0, 0, 0, z_pos1, 1, z_pos2, 1, 0, 0, 0, 0, z_pos2, 1,
      z_pos3, 1, 0, 0, 0, 0, z_pos3, 1;

  // Fill the d vector with x_ and y_ values
  d_vec << x_pos1, y_pos1, x_pos2, y_pos2, x_pos3, y_pos3;

  // Fill the weights vector
  w_vec = Eigen::Matrix<double, 6, 1>(weights.data());

  // Solve the weighted least squares system
  Eigen::MatrixXd At_W_A = A_mat.transpose() * w_vec.asDiagonal() * A_mat;
  Eigen::MatrixXd At_W_d = A_mat.transpose() * w_vec.asDiagonal() * d_vec;
  Eigen::VectorXd param_vec = At_W_A.ldlt().solve(At_W_d);

  Eigen::Matrix4d covariance_matrix = At_W_A.inverse();

  // Store only the upper triangular part of the covariance matrix since it is
  // symmetric
  std::vector<double> covariance_vector = {
      covariance_matrix(0, 0), covariance_matrix(0, 1), covariance_matrix(0, 2),
      covariance_matrix(0, 3), covariance_matrix(1, 1), covariance_matrix(1, 2),
      covariance_matrix(1, 3), covariance_matrix(2, 2), covariance_matrix(2, 3),
      covariance_matrix(3, 3)};

  // return {slope_x, intercept_x, slope_y, intercept_y, covariance}
  return {param_vec(0), param_vec(1), param_vec(2), param_vec(3),
          covariance_vector};
}  // fit3DLine

double LinearSeedFinder::calculateDistance(
    const std::array<double, 3>& point1, const std::array<double, 3>& point2) {
  return sqrt(pow(point1[1] - point2[1], 2) + pow(point1[2] - point2[2], 2));
}  // calculateDistance in xy

double LinearSeedFinder::globalChiSquare(
    const std::array<double, 3>& first_sensor,
    const std::array<double, 3>& second_sensor,
    const std::array<double, 3>& ecal_hit, double m_x, double m_y, double b_x,
    double b_y) {
  double chi2_x = 0, chi2_y = 0;
  chi2_x += pow(
      (m_x * first_sensor[0] + b_x - first_sensor[1]) / recoil_uncertainty_[0],
      2);
  chi2_y += pow(
      (m_y * first_sensor[0] + b_y - first_sensor[2]) / recoil_uncertainty_[1],
      2);

  chi2_x += pow((m_x * second_sensor[0] + b_x - second_sensor[1]) /
                    recoil_uncertainty_[0],
                2);
  chi2_y += pow((m_y * second_sensor[0] + b_y - second_sensor[2]) /
                    recoil_uncertainty_[1],
                2);

  chi2_x += pow((m_x * ecal_hit[0] + b_x - ecal_hit[1]) / ecal_uncertainty_, 2);
  chi2_y += pow((m_y * ecal_hit[0] + b_y - ecal_hit[2]) / ecal_uncertainty_, 2);

  return chi2_x + chi2_y;
}  // globalChiSquare

int LinearSeedFinder::uniqueLayersHit(
    const std::vector<ldmx::Measurement>& digi_points) {
  std::vector<ldmx::Measurement> sorted_points = digi_points;

  // Sort by z_ position in the Recoil
  std::sort(sorted_points.begin(), sorted_points.end(),
            [](const ldmx::Measurement& meas1, const ldmx::Measurement& meas2) {
              return meas1.getGlobalPosition()[0] <
                     meas2.getGlobalPosition()[0];
            });

  // Remove duplicates to ensure we only keep unique z_ positions
  auto last = std::unique(
      sorted_points.begin(), sorted_points.end(),
      [](const ldmx::Measurement& meas1, const ldmx::Measurement& meas2) {
        return meas1.getGlobalPosition()[0] == meas2.getGlobalPosition()[0];
      });

  // return the number of unique layer_ hits_
  return std::distance(sorted_points.begin(), last);
}  // uniqueLayersHit

}  // namespace reco
}  // namespace tracking

DECLARE_PRODUCER(tracking::reco::LinearSeedFinder);

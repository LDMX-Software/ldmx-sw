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

  // Input strip hits
  input_hits_collection_ = parameters.getParameter<std::string>(
      "input_hits_collection", "DigiRecoilSimHits");
  input_rec_hits_collection_ = parameters.getParameter<std::string>(
      "input_rec_hits_collection", "EcalRecHits");

  input_pass_name_ =
      parameters.getParameter<std::string>("input_pass_name", "");

  // the uncertainty is sigma_x = 6 microns and sigma_y = 20./sqrt(12)
  recoil_uncertainty_ = parameters.getParameter<std::vector<double>>(
      "recoil_uncertainty", {0.006, 5.7735});
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

  // Find RecHits at first layer of ECal
  for (const auto& x_ecal : ecal_rec_hit) {
    if (x_ecal.getZPos() < ecal_first_layer_z_threshold_) {
      first_layer_ecal_rec_hits.push_back(
          {x_ecal.getZPos(), x_ecal.getXPos(), x_ecal.getYPos()});
    }  // if first layer of Ecal
  }    // for positions in ecalRecHit

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
  if (event.exists("SimParticles")) {
    particle_map = event.getMap<int, ldmx::SimParticle>("SimParticles");
    truth_matching_tool_->setup(particle_map, recoil_hits);
  }
    
    std::vector<ldmx::Measurement> first_two_layers;
    for (const auto& point : recoil_hits) {
      if (point.getGlobalPosition()[0] < layer12_midpoint_)
        first_two_layers.push_back(point);
      else if (point.getGlobalPosition()[0] < layer23_midpoint_)
        first_two_layers.push_back(point);
      else
        continue;
    }


    auto firstSensorCombos = processMeasurements(first_two_layers, tg);
      for (const auto& combo : firstSensorCombos) {
          auto [combo_3d_points, first_layer, second_layer] = combo;
          
          ldmx_log(debug) << "The combined 3D hit is: (" << combo_3d_points[0] << ", "
          << combo_3d_points[1] << ", " << combo_3d_points[2] << ")\n";
          ldmx_log(debug) << "which is made out of layer1= (" << first_layer.getGlobalPosition()[0] << ", "
          << first_layer.getGlobalPosition()[1] << ", " << first_layer.getGlobalPosition()[2] << ")\n";
          ldmx_log(debug) << "and is made out of layer2= (" << second_layer.getGlobalPosition()[0] << ", "
          << second_layer.getGlobalPosition()[1] << ", " << second_layer.getGlobalPosition()[2] << ")\n";
          
          const std::vector<ldmx::SimTrackerHit> recoil_sim_hits = event.getCollection<ldmx::SimTrackerHit>("RecoilSimHits",
                                                                                                            input_pass_name_);
          
          ldmx_log(debug) << "The first recoil layer trackID is: (" << first_layer.getTrackIds()[0] << ")\n";
          ldmx_log(debug) << "The second recoil layer trackID is: (" << second_layer.getTrackIds()[0] << ")\n";

          for (const auto& hit : recoil_sim_hits) {
              ldmx_log(debug) << "The hit trackID is: (" << hit.getTrackID() << ")\n";
              if (hit.getTrackID() == first_layer.getTrackIds()[0]) {
                  ldmx_log(debug) << "The global position of the SimParticle with matching trackID to the measurement is: (" << hit.getPosition()[0] << ", " << hit.getPosition()[1] << ", " << hit.getPosition()[2] << ")\n";
              }
          }
          
      }

// weighted averaging: layer1+layer2 = sensor1, layer3+layer4 = sensor2
  // this gives all possible combinations of two tracker points for fitting
  auto [first_sensor, second_sensor] = combineMultiGlobalHits(recoil_hits);

  for (const auto& first_point : first_sensor) {
    for (const auto& second_point : second_sensor) {
      for (const auto& rec_hit : first_layer_ecal_rec_hits) {
        // Do fitting on 2 sensor + 1 recHit combinations = 1 degree of freedom
        // for linear fit
        ldmx::StraightTrack seed_track =
            SeedTracker(first_point, second_point, rec_hit);

        // Seed passed RecHit distance check
        if (seed_track.getChi2() > 0.0) {
          straight_seed_tracks.push_back(seed_track);
        }  // if chi2 > 0
      }    // for rec_hits
    }      // for second recoil tracker
  }        // for first recoil tracker

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

  // TODO: in the case where we don't have all 4 hits, we will be fitting a
  // sensor (weighted average of two layers) + single layer
  // TODO: or fitting two single layers. Currently, the single layer point has
  // the uncertainty of a sensor assigned to it,
  // TODO: but this is not a realistic uncertainty for a single layer...
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

  // Fit the 3 points to a 3D straight line, find track location at first layer
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

std::pair<std::vector<std::tuple<std::array<double, 3>, ldmx::Measurement,
                                 std::optional<ldmx::Measurement>>>,
          std::vector<std::tuple<std::array<double, 3>, ldmx::Measurement,
                                 std::optional<ldmx::Measurement>>>>
LinearSeedFinder::combineMultiGlobalHits(
    const std::vector<ldmx::Measurement>& hit_collection) {
  std::vector<ldmx::Measurement> layer1, layer2, layer3, layer4;

  // Split hits into layers based on z position
  // TODO: can we access layer information directly from ldmx::Measurement??
  for (const auto& point : hit_collection) {
    if (point.getGlobalPosition()[0] < layer12_midpoint_)
      layer1.push_back(point);
    else if (point.getGlobalPosition()[0] < layer23_midpoint_)
      layer2.push_back(point);
    else if (point.getGlobalPosition()[0] < layer34_midpoint_)
      layer3.push_back(point);
    else
      layer4.push_back(point);
  }

  std::vector<std::tuple<std::array<double, 3>, ldmx::Measurement,
                         std::optional<ldmx::Measurement>>>
      first_sensor_merged_hits, second_sensor_merged_hits;

  if (layer1.empty()) {
    for (const auto& point : layer2) {
      first_sensor_merged_hits.push_back(
          std::make_tuple(std::array<double, 3>{point.getGlobalPosition()[0],
                                                point.getGlobalPosition()[1],
                                                point.getGlobalPosition()[2]},
                          point, std::nullopt));
    }  // only look at layer2
  }    // if layer1 empty
  else if (layer2.empty()) {
    for (const auto& point : layer1) {
      first_sensor_merged_hits.push_back(
          std::make_tuple(std::array<double, 3>{point.getGlobalPosition()[0],
                                                point.getGlobalPosition()[1],
                                                point.getGlobalPosition()[2]},
                          point, std::nullopt));
    }  // only look at layer1
  }    // if layer2 empty
  else {
    first_sensor_merged_hits = midPointCalculation(layer1, layer2);
  }  // do weighted average of two layers

  if (layer3.empty()) {
    for (const auto& point : layer4) {
      second_sensor_merged_hits.push_back(
          std::make_tuple(std::array<double, 3>{point.getGlobalPosition()[0],
                                                point.getGlobalPosition()[1],
                                                point.getGlobalPosition()[2]},
                          point, std::nullopt));
    }  // only look at layer4
  }    // if layer3 empty
  else if (layer4.empty()) {
    for (const auto& point : layer3) {
      second_sensor_merged_hits.push_back(
          std::make_tuple(std::array<double, 3>{point.getGlobalPosition()[0],
                                                point.getGlobalPosition()[1],
                                                point.getGlobalPosition()[2]},
                          point, std::nullopt));
    }
  }  // if layer4 empty
  // do weighted average of two layers
  else {
    second_sensor_merged_hits = midPointCalculation(layer3, layer4);
  }

  return {first_sensor_merged_hits, second_sensor_merged_hits};
}  // combineMultiGlobalHits

std::vector<std::tuple<std::array<double, 3>, ldmx::Measurement, ldmx::Measurement>> LinearSeedFinder::processMeasurements(const std::vector<ldmx::Measurement>& measurements, const geo::TrackersTrackingGeometry& tg) {
    
    std::vector<ldmx::Measurement> axialMeasurements;
    std::vector<ldmx::Measurement> stereoMeasurements;
    std::vector<std::tuple<std::array<double, 3>, ldmx::Measurement, ldmx::Measurement>> points_with_measurement;
    
    for (const auto& meas : measurements) {
        if (meas.getLayerID() % 2 == 0) {
            axialMeasurements.push_back(meas);
        } // even layers are axial, from looking at Measurement.h
        else {
            stereoMeasurements.push_back(meas);
        } // odd layers are stereo, from looking at Meaasurement.h
    } // for measurements
    
    for (const auto& axial : axialMeasurements) {
        for (const auto& stereo : stereoMeasurements) {
            
            // Get surfaces from ACTS
            const Acts::Surface* axial_surface = tg.getSurface(axial.getLayerID());
            const Acts::Surface* stereo_surface = tg.getSurface(stereo.getLayerID());
            
            if (!axial_surface || !stereo_surface) continue;  // Skip invalid surfaces
            
            // Compute 3D space point
            Acts::Vector3 spacePoint = compute3DHit(axial, *axial_surface, stereo, *stereo_surface);
            
            points_with_measurement.push_back({convertToLdmxStdArray(spacePoint), axial, stereo});
        } // for stereo
    } // for axial
    
    return points_with_measurement;
}

//I think ACTS saves its arrays like (x, y, z)
std::array<double, 3> LinearSeedFinder::convertToLdmxStdArray(const Acts::Vector3& vec) {
    return {vec.x(), vec.y(), vec.z()};
}

//Helper function to calculate unit vector by taking advantage of the localToGlobal transformation
std::tuple<Acts::Vector3, Acts::Vector3, Acts::Vector3> LinearSeedFinder::getSurfaceVectors(const Acts::Surface& surface) {
    Acts::Vector3 dummy{0., 0., 0.};
    Acts::Vector3 u = surface.localToGlobal(geometry_context(), Acts::Vector2(1, 0), dummy) - surface.center(geometry_context());
    Acts::Vector3 v = surface.localToGlobal(geometry_context(), Acts::Vector2(0, 1), dummy) - surface.center(geometry_context());
    Acts::Vector3 w = u.cross(v).normalized();
    return {u.normalized(), v.normalized(), w};
}

Acts::Vector3 LinearSeedFinder::compute3DHit(const ldmx::Measurement& axial, const Acts::Surface& axial_surface, const ldmx::Measurement& stereo, const Acts::Surface& stereo_surface) {
    
    // Compute unit vectors for both hits
    auto [axial_u, axial_v, axial_w] = getSurfaceVectors(axial_surface);
    auto [stereo_u, stereo_v, stereo_w] = getSurfaceVectors(stereo_surface);

    ldmx_log(debug) << "############################################################" << "\n";
    ldmx_log(debug) << "The axial unit vectors are: " << "\n";
    ldmx_log(debug) << "u: " << axial_u[0] << ", " << axial_u[1] << ", " << axial_u[2] << "\n";
    ldmx_log(debug) << "v: " << axial_v[0] << ", " << axial_v[1] << ", " << axial_v[2] << "\n";
    ldmx_log(debug) << "w: " << axial_w[0] << ", " << axial_w[1] << ", " << axial_w[2] << "\n";

    ldmx_log(debug) << "The stereo unit vectors are: " << "\n";
    ldmx_log(debug) << "u: " << stereo_u[0] << ", " << stereo_u[1] << ", " << stereo_u[2] << "\n";
    ldmx_log(debug) << "v: " << stereo_v[0] << ", " << stereo_v[1] << ", " << stereo_v[2] << "\n";
    ldmx_log(debug) << "w: " << stereo_w[0] << ", " << stereo_w[1] << ", " << stereo_w[2] << "\n";

    // Get global positions for strip origins
    Acts::Vector3 axial_origin = axial_surface.center(geometry_context());
    Acts::Vector3 stereo_origin = stereo_surface.center(geometry_context());
    
    ldmx_log(debug) << "The axial global origins are: " << axial_origin[0] << ", " << axial_origin[1] << ", " << axial_origin[2] << "\n";
    ldmx_log(debug) << "The stereo global origins are: " << stereo_origin[0] << ", " << stereo_origin[1] << ", " << stereo_origin[2] << "\n";
    
    // Get local position components
    auto [axial_u_value, axial_v_value] = axial.getLocalPosition();
    auto [stereo_u_value, stereo_v_value] = stereo.getLocalPosition();
    
    // Manual correction, since v should always be 0 (insensitive direction)
    axial_v_value = 0.0;
    stereo_v_value = 0.0;
    
    ldmx_log(debug) << "The axial local positions are: " << axial_u_value << ", " << axial_v_value << "\n";
    ldmx_log(debug) << "The stereo local positions are: " << stereo_u_value << ", " << stereo_v_value << "\n";
    
    Acts::Vector3 dummy{0., 0., 0.};
    Acts::Vector3 axial_global = axial_surface.localToGlobal(geometry_context(), Acts::Vector2(axial_u_value, axial_v_value), dummy);
    Acts::Vector3 stereo_global = stereo_surface.localToGlobal(geometry_context(), Acts::Vector2(stereo_u_value, stereo_v_value), dummy);
    
    ldmx_log(debug) << "The axial localToGlobal (i.e. global position) are: " << axial_global[0] << ", " << axial_global[1] << ", " << axial_global[2] << "\n";
    ldmx_log(debug) << "The stereo localToGlobal (i.e. global position) are: " << stereo_global[0] << ", " << stereo_global[1] << ", " << stereo_global[2] << "\n";
    
    // From here we follow the logic of the HPS code
    double gamma = dotProduct(stereo_origin, stereo_w) / nonZeroDotProduct(axial_origin, axial_w);
    ldmx_log(debug) << "The value of gamma is: " << gamma << "\n";
    
    //This should be equivalent to the sin of the stereo angle, according to HPS
    double salpha = dotProduct(axial_v, stereo_u);
    ldmx_log(debug) << "The value of salpha is: " << salpha << "\n";

    Acts::Vector3 p1 = stripCenter(axial_origin, axial_u_value, axial_u); //axial_u = axial unit vector in u
    ldmx_log(debug) << "The p1 vector is: [" << p1[0] << ", " << p1[1] << ", " << p1[2] << "] \n";

    Acts::Vector3 p2 = stripCenter(stereo_origin, stereo_u_value, stereo_u); //stereo_u = stereo unit vector in u
    ldmx_log(debug) << "The p2 vector is: [" << p2[0] << ", " << p2[1] << ", " << p2[2] << "] \n";

    Acts::Vector3 dp = p2 - p1;
    ldmx_log(debug) << "The dp vector is: [" << dp[0] << ", " << dp[1] << ", " << dp[2] << "] \n";

    double v1 = dotProduct(dp, stereo_u) / (gamma*salpha);
    ldmx_log(debug) << "The value of v1 is: " << v1 << "\n";

    if (v1 < -50.0) { v1 = -50.0; }
    else if (v1 > 50.0) { v1 = 50.0; }
    
    Acts::Vector3 r1 = p1 + v1 * axial_v; //axial_v = axial unit vector in v
    ldmx_log(debug) << "The r1 vector is: [" << r1[0] << ", " << r1[1] << ", " << r1[2] << "] \n";
    
    Acts::Vector3 final_position = (0.5 * (1 + gamma)) * r1;
    ldmx_log(debug) << "The final position is: [" << final_position[0] << ", " << final_position[1] << ", " << final_position[2] << "] \n";
    ldmx_log(debug) << "########################## end ##################################" << "\n";

    return final_position;
}

double LinearSeedFinder::dotProduct(const Acts::Vector3& v1, const Acts::Vector3& v2) {
    return v1.dot(v2);
}

Acts::Vector3 LinearSeedFinder::stripCenter(const Acts::Vector3& strip_origin, double u, const Acts::Vector3& strip_uhat) {
    return strip_origin + u * strip_uhat;
}

double LinearSeedFinder::nonZeroDotProduct(const Acts::Vector3& v1, const Acts::Vector3& v2) {
    double cth = v1.dot(v2);
    double eps = 1e-6;
    if (std::abs(cth) < eps) {
        cth = (cth < 0.0) ? -eps : eps;
    }
    return cth;
}


std::vector<std::tuple<std::array<double, 3>, ldmx::Measurement,
                       std::optional<ldmx::Measurement>>>
LinearSeedFinder::midPointCalculation(
    const std::vector<ldmx::Measurement>& layer1,
    const std::vector<ldmx::Measurement>& layer2) {
  std::vector<std::tuple<std::array<double, 3>, ldmx::Measurement,
                         std::optional<ldmx::Measurement>>>
      merged_hits;

  for (const auto& point1 : layer1) {
    for (const auto& point2 : layer2) {
      double z_avg =
          (point1.getGlobalPosition()[0] + point2.getGlobalPosition()[0]) /
          (2.0);
      double x_avg =
          (point1.getGlobalPosition()[1] + point2.getGlobalPosition()[1]) /
          (2.0);
      // Until we make axial/stereo combinations, we don't know anything about
      // the y value
      double y_avg = 0.0;

      merged_hits.push_back(std::make_tuple(
          std::array<double, 3>{z_avg, x_avg, y_avg}, point1, point2));

    }  // for layer2
  }    // for layer1
  return merged_hits;
}  // midPointCalculation

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

  // Fill the A matrix (z, 1, 0, 0) for x and (0, 0, z, 1) for y
  A_mat << z_pos1, 1, 0, 0, 0, 0, z_pos1, 1, z_pos2, 1, 0, 0, 0, 0, z_pos2, 1,
      z_pos3, 1, 0, 0, 0, 0, z_pos3, 1;

  // Fill the d vector with x and y values
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

  // Sort by z position in the Recoil
  std::sort(sorted_points.begin(), sorted_points.end(),
            [](const ldmx::Measurement& meas1, const ldmx::Measurement& meas2) {
              return meas1.getGlobalPosition()[0] <
                     meas2.getGlobalPosition()[0];
            });

  // Remove duplicates to ensure we only keep unique z positions
  auto last = std::unique(
      sorted_points.begin(), sorted_points.end(),
      [](const ldmx::Measurement& meas1, const ldmx::Measurement& meas2) {
        return meas1.getGlobalPosition()[0] == meas2.getGlobalPosition()[0];
      });

  // return the number of unique layer hits
  return std::distance(sorted_points.begin(), last);
}  // uniqueLayersHit

}  // namespace reco
}  // namespace tracking

DECLARE_PRODUCER_NS(tracking::reco, LinearSeedFinder);

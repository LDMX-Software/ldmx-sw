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

  recoil_uncertainty_ = parameters.getParameter<std::vector<double>>(
      "recoil_uncertainty", {0.006, 0.12});
  ecal_uncertainty_ =
      parameters.getParameter<double>("ecal_uncertainty", {3.87});
  ecal_distance_threshold_ =
      parameters.getParameter<double>("ecal_distance_threshold");

  layer12_midpoint_ = parameters.getParameter<double>("layer12_midpoint");
  layer23_midpoint_ = parameters.getParameter<double>("layer23_midpoint");
  layer34_midpoint_ = parameters.getParameter<double>("layer34_midpoint");
}

void LinearSeedFinder::produce(framework::Event& event) {
  auto start = std::chrono::high_resolution_clock::now();
  std::vector<ldmx::StraightTrack> straight_seed_tracks;
  n_events_++;

  const std::vector<ldmx::Measurement> recoil_hits =
      event.getCollection<ldmx::Measurement>(input_hits_collection_);
  const std::vector<ldmx::EcalHit> ecal_rec_hit =
      event.getCollection<ldmx::EcalHit>(input_rec_hits_collection_);

  std::vector<std::array<double, 3>> first_layer_ecal_rec_hits;

  // Find RecHits at first layer of ECal
  for (const auto& x_ecal : ecal_rec_hit) {
    if (x_ecal.getZPos() < 250) {
      first_layer_ecal_rec_hits.push_back(
          {x_ecal.getZPos(), x_ecal.getXPos(), x_ecal.getYPos()});
    }  // if first layer of Ecal
  }    // for positions in ecalRecHit

  // Check if we would fit empty seeds, if so: end tracking
  if (recoil_hits.size() < 2 || first_layer_ecal_rec_hits.empty() ||
      uniqueLayersHit(recoil_hits) < 2) {
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
        if (seed_track.getChi2() > 0.0) {
          straight_seed_tracks.push_back(
              seed_track);  // Seed passed RecHit distance check
        }
      }  // for rec_hits
    }    // for second recoil tracker
  }      // for first recoil tracker

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

  if (layer2.has_value()) {
    all_points.push_back(*layer2);
  }  // if layer2 doesn't exist (has_value == False), then the layer1 we added
     // is either layer1 or 2, depending on which one has_value

  all_points.push_back(layer3);

  if (layer4.has_value()) {
    all_points.push_back(*layer4);
  }  // if layer4 doesn't exist (has_value == False), then the layer3 we added
     // is either layer3 or 4, depending on which one has_value

  // Fit the 3 points to a 3D straight line, find track location at first layer
  // of Ecal, check distance to recHit used in fitting
  auto [mx, bx, my, by, seed_cov] =
      fit3DLine(sensor1, sensor2, ecal_one);  // m = slope ; b = intercept
  std::array<double, 3> temp_extrapolated_point = {
      ecal_one[0], mx * ecal_one[0] + bx, my * ecal_one[0] + by};
  double temp_distance = calculateDistance(temp_extrapolated_point, ecal_one);

  ldmx::StraightTrack trk = ldmx::StraightTrack();

  if (temp_distance < ecal_distance_threshold_) {
    trk.setSlopeX(mx);
    trk.setInterceptX(bx);
    trk.setSlopeY(my);
    trk.setInterceptY(by);
    trk.setTheta(std::atan2(my, std::sqrt(1 + mx * mx)));
    trk.setPhi(std::atan2(mx, 1.0));

    trk.setAllSensorPoints(all_points);
    trk.setFirstSensorPosition(sensor1);
    trk.setSecondSensorPosition(sensor2);
    trk.setFirstLayerEcalRecHit(ecal_one);
    trk.setDistancetoRecHit(temp_distance);

    trk.setTargetLocation(0.0, bx, by);
    trk.setEcalLayer1Location(temp_extrapolated_point);
    trk.setChi2(globalChiSquare(sensor1, sensor2, ecal_one, mx, my, bx, by));
    trk.setNhits(3);
    trk.setNdf(1);

    trk.setCov(seed_cov);

    if (truth_matching_tool_->configured()) {
      auto truth_info = truth_matching_tool_->TruthMatch(all_points);
      trk.setTrackID(truth_info.trackID);
      trk.setPdgID(truth_info.pdgID);
      trk.setTruthProb(truth_info.truthProb);
    }  // truthMatching

    return trk;

  }  // check whether the track is close enough to EcalRecHit
  else {
    trk.setChi2(-1);
    return trk;
  }  // does not pass the threshold
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
    for (const auto& p : layer2) {
      first_sensor_merged_hits.push_back(
          std::make_tuple(std::array<double, 3>{p.getGlobalPosition()[0],
                                                p.getGlobalPosition()[1],
                                                p.getGlobalPosition()[2]},
                          p, std::nullopt));
    }  // only look at layer2
  }    // if layer1 empty
  else if (layer2.empty()) {
    for (const auto& p : layer1) {
      first_sensor_merged_hits.push_back(
          std::make_tuple(std::array<double, 3>{p.getGlobalPosition()[0],
                                                p.getGlobalPosition()[1],
                                                p.getGlobalPosition()[2]},
                          p, std::nullopt));
    }  // only look at layer1
  }    // if layer2 empty
  else {
    first_sensor_merged_hits = weightedAverage(layer1, layer2);
  }  // do weighted average of two layers

  if (layer3.empty()) {
    for (const auto& p : layer4) {
      second_sensor_merged_hits.push_back(
          std::make_tuple(std::array<double, 3>{p.getGlobalPosition()[0],
                                                p.getGlobalPosition()[1],
                                                p.getGlobalPosition()[2]},
                          p, std::nullopt));
    }  // only look at layer4
  }    // if layer3 empty
  else if (layer4.empty()) {
    for (const auto& p : layer3) {
      second_sensor_merged_hits.push_back(
          std::make_tuple(std::array<double, 3>{p.getGlobalPosition()[0],
                                                p.getGlobalPosition()[1],
                                                p.getGlobalPosition()[2]},
                          p, std::nullopt));
    }  // only look at layer4
  }    // if layer4 empty
  else {
    second_sensor_merged_hits = weightedAverage(layer3, layer4);
  }  // do weighted average of two layers

  return {first_sensor_merged_hits, second_sensor_merged_hits};
}  // combineMultiGlobalHits

std::vector<std::tuple<std::array<double, 3>, ldmx::Measurement,
                       std::optional<ldmx::Measurement>>>
LinearSeedFinder::weightedAverage(
    const std::vector<ldmx::Measurement>& layer1,
    const std::vector<ldmx::Measurement>& layer2) {
  std::vector<std::tuple<std::array<double, 3>, ldmx::Measurement,
                         std::optional<ldmx::Measurement>>>
      merged_hits;

  for (const auto& p1 : layer1) {
    for (const auto& p2 : layer2) {
      double edepL1 = p1.getEdep();
      double edepL2 = p2.getEdep();
      double z_avg = (p1.getGlobalPosition()[0] * edepL1 +
                      p2.getGlobalPosition()[0] * edepL2) /
                     (edepL1 + edepL2);
      double x_avg = (p1.getGlobalPosition()[1] * edepL1 +
                      p2.getGlobalPosition()[1] * edepL2) /
                     (edepL1 + edepL2);
      double y_avg = (p1.getGlobalPosition()[2] * edepL1 +
                      p2.getGlobalPosition()[2] * edepL2) /
                     (edepL1 + edepL2);
      merged_hits.push_back(
          std::make_tuple(std::array<double, 3>{z_avg, x_avg, y_avg}, p1, p2));
    }
  }
  return merged_hits;
}  // weightedAverage

std::tuple<double, double, double, double, std::vector<double>>
LinearSeedFinder::fit3DLine(const std::array<double, 3>& first_recoil,
                            const std::array<double, 3>& second_recoil,
                            const std::array<double, 3>& ecal) {
  double z1 = first_recoil[0], x1 = first_recoil[1], y1 = first_recoil[2];
  double z2 = second_recoil[0], x2 = second_recoil[1], y2 = second_recoil[2];
  double z3 = ecal[0], x3 = ecal[1], y3 = ecal[2];

  std::array<double, 6> weights = {
      1 / pow(recoil_uncertainty_[0], 2), 1 / pow(recoil_uncertainty_[1], 2),
      1 / pow(recoil_uncertainty_[0], 2), 1 / pow(recoil_uncertainty_[1], 2),
      1 / pow(ecal_uncertainty_, 2),      1 / pow(ecal_uncertainty_, 2)};

  Eigen::Matrix<double, 6, 4> A;
  Eigen::Matrix<double, 6, 1> d, w;

  // Fill the A matrix (z, 1, 0, 0) for x and (0, 0, z, 1) for y
  A << z1, 1, 0, 0, 0, 0, z1, 1, z2, 1, 0, 0, 0, 0, z2, 1, z3, 1, 0, 0, 0, 0,
      z3, 1;

  // Fill the d vector with x and y values
  d << x1, y1, x2, y2, x3, y3;

  // Fill the weights vector
  w = Eigen::Matrix<double, 6, 1>(weights.data());

  // Solve the weighted least squares system
  Eigen::MatrixXd At_W_A = A.transpose() * w.asDiagonal() * A;
  Eigen::MatrixXd At_W_d = A.transpose() * w.asDiagonal() * d;
  Eigen::VectorXd m = At_W_A.ldlt().solve(At_W_d);

  Eigen::Matrix4d covariance_matrix = At_W_A.inverse();

  // Store only the upper triangular part of the covariance matrix since it is
  // symmetric
  std::vector<double> covariance_vector = {
      covariance_matrix(0, 0), covariance_matrix(0, 1), covariance_matrix(0, 2),
      covariance_matrix(0, 3), covariance_matrix(1, 1), covariance_matrix(1, 2),
      covariance_matrix(1, 3), covariance_matrix(2, 2), covariance_matrix(2, 3),
      covariance_matrix(3, 3)};

  return {m(0), m(1), m(2), m(3),
          covariance_vector};  // return {slope_x, intercept_x, slope_y,
                               // intercept_y, covariance}
}  // fit3DLine

double LinearSeedFinder::calculateDistance(
    const std::array<double, 3>& point1, const std::array<double, 3>& point2) {
  return sqrt(pow(point1[1] - point2[1], 2) + pow(point1[2] - point2[2], 2));
}  // calculateDistance in xy

double LinearSeedFinder::globalChiSquare(
    const std::array<double, 3>& first_sensor,
    const std::array<double, 3>& second_sensor,
    const std::array<double, 3>& ecal_hit, double mx, double my, double bx,
    double by) {
  double chi2_x = 0, chi2_y = 0;
  chi2_x += pow(
      (mx * first_sensor[0] + bx - first_sensor[1]) / recoil_uncertainty_[0],
      2);
  chi2_y += pow(
      (my * first_sensor[0] + by - first_sensor[2]) / recoil_uncertainty_[1],
      2);

  chi2_x += pow(
      (mx * second_sensor[0] + bx - second_sensor[1]) / recoil_uncertainty_[0],
      2);
  chi2_y += pow(
      (my * second_sensor[0] + by - second_sensor[2]) / recoil_uncertainty_[1],
      2);

  chi2_x += pow((mx * ecal_hit[0] + bx - ecal_hit[1]) / ecal_uncertainty_, 2);
  chi2_y += pow((my * ecal_hit[0] + by - ecal_hit[2]) / ecal_uncertainty_, 2);

  return chi2_x + chi2_y;
}  // globalChiSquare

int LinearSeedFinder::uniqueLayersHit(
    const std::vector<ldmx::Measurement>& digi_points) {
  std::vector<ldmx::Measurement> sorted_points = digi_points;

  // Sort by z-position in the Recoil
  std::sort(sorted_points.begin(), sorted_points.end(),
            [](const ldmx::Measurement& m1, const ldmx::Measurement& m2) {
              return m1.getGlobalPosition()[0] < m2.getGlobalPosition()[0];
            });

  // Remove duplicates to ensure we only keep unique z positions
  auto last = std::unique(
      sorted_points.begin(), sorted_points.end(),
      [](const ldmx::Measurement& m1, const ldmx::Measurement& m2) {
        return m1.getGlobalPosition()[0] == m2.getGlobalPosition()[0];
      });

  return std::distance(sorted_points.begin(),
                       last);  // return the number of unique layer hits
}  // uniqueLayersHit

}  // namespace reco
}  // namespace tracking

DECLARE_PRODUCER_NS(tracking::reco, LinearSeedFinder);

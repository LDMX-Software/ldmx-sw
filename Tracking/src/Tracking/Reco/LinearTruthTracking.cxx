#include "Tracking/Reco/LinearTruthTracking.h"

#include "Eigen/Dense"

namespace tracking {
namespace reco {

LinearTruthTracking::LinearTruthTracking(const std::string& name,
                                         framework::Process& process)
    : TrackingGeometryUser(name, process) {}

void LinearTruthTracking::onProcessStart() {
  truth_matching_tool_ = std::make_shared<tracking::sim::TruthMatchingTool>();
}

void LinearTruthTracking::configure(framework::config::Parameters& parameters) {
  // Output seed name
  out_trk_collection_ = parameters.getParameter<std::string>(
      "out_trk_collection", "LinearRecoilTruthTracks");

  // Input strip hits
  input_hits_collection_ = parameters.getParameter<std::string>(
      "input_hits_collection", "DigiRecoilSimHits");
  input_rec_hits_collection_ = parameters.getParameter<std::string>(
      "input_recHits_collection", "EcalRecHits");

  layer12_midpoint_ = parameters.getParameter<double>("layer12_midpoint");
  layer23_midpoint_ = parameters.getParameter<double>("layer23_midpoint");
  layer34_midpoint_ = parameters.getParameter<double>("layer34_midpoint");
}

void LinearTruthTracking::produce(framework::Event& event) {
  auto start = std::chrono::high_resolution_clock::now();
  std::vector<ldmx::StraightTrack> straight_truth_tracks;
  n_events_++;

  const std::vector<ldmx::Measurement> recoil_hits =
      event.getCollection<ldmx::Measurement>(input_hits_collection_);
  const std::vector<ldmx::EcalHit> ecal_rec_hit =
      event.getCollection<ldmx::EcalHit>(input_rec_hits_collection_);

  std::vector<std::array<double, 3>> first_layer_ecal_rec_hits;

  for (const auto& x_ecal : ecal_rec_hit) {
    if (x_ecal.getZPos() < 250) {
      first_layer_ecal_rec_hits.push_back(
          {x_ecal.getZPos(), x_ecal.getXPos(), x_ecal.getYPos()});
    }  // if first layer of Ecal
  }    // for positions in ecalRecHit

  std::map<int, ldmx::SimParticle> particle_map;
  if (event.exists("SimParticles")) {
    particle_map = event.getMap<int, ldmx::SimParticle>("SimParticles");
    truth_matching_tool_->setup(particle_map, recoil_hits);
  }

  // Check if we would fit empty seeds.
  // TODO: Currently, we only want more than 2 hits in recoil. Is there a better
  // check?
  if (recoil_hits.size() < 2) {
    n_missing_++;
    n_truth_ += straight_truth_tracks.size();
    event.add(out_trk_collection_, straight_truth_tracks);
    return;
  }

  std::vector<ldmx::Measurement> truth_points;

  if (truth_matching_tool_->configured()) {
    truth_points = findTruthHits(recoil_hits);
  }  // match recoilHits with truth data

  if (truth_points.size() > 0) {
    straight_truth_tracks.push_back(
        truthTracker(truth_points, first_layer_ecal_rec_hits));
  }  // should only find 1 track (recoil e-) or 0 track (no recoil e-)

  else {
    n_empty_++;
    n_truth_ += straight_truth_tracks.size();
    return;
  }

  n_truth_ += straight_truth_tracks.size();
  event.add(out_trk_collection_, straight_truth_tracks);

  auto end = std::chrono::high_resolution_clock::now();
  auto diff = end - start;
  processing_time_ += std::chrono::duration<double, std::milli>(diff).count();

  first_layer_ecal_rec_hits.clear();
  straight_truth_tracks.clear();

}  // produce

ldmx::StraightTrack LinearTruthTracking::truthTracker(
    const std::vector<ldmx::Measurement>& points,
    std::vector<std::array<double, 3>>& ecal_points) {
  ldmx::StraightTrack trk = ldmx::StraightTrack();

  // We have pre-selected hits corresponding to the Recoil e-, so we only fit
  // these (no RecHit)
  auto [mx, bx, my, by, truth_trk_cov] = fit3DLine(points);

  trk.setSlopeX(mx);
  trk.setInterceptX(bx);
  trk.setSlopeY(my);
  trk.setInterceptY(by);
  trk.setTheta(std::atan2(my, std::sqrt(1 + mx * mx)));
  trk.setPhi(std::atan2(mx, 1.0));
  trk.setTargetLocation(0.0, bx, by);
  trk.setNhits(points.size());
  trk.setNdf(points.size() - 2);

  if (truth_matching_tool_->configured()) {
    auto truth_info = truth_matching_tool_->TruthMatch(points);
    trk.setTrackID(truth_info.trackID);
    trk.setPdgID(truth_info.pdgID);
    trk.setTruthProb(truth_info.truthProb);
  }

  trk.setAllSensorPoints(points);

  trk.setChi2(globalChiSquare(points, mx, my, bx, by));
  trk.setCov(truth_trk_cov);

  // Z position from the first point in ecalPoints
  if (ecal_points.size() > 0) {
    double ecal_first_layer_z = ecal_points[0][0];

    // Extrapolate track to first layer of Ecal
    std::array<double, 3> extrapolated_point = {ecal_first_layer_z,
                                                mx * ecal_first_layer_z + bx,
                                                my * ecal_first_layer_z + by};

    const std::array<double, 3>* closest_rec_hit = nullptr;
    double min_distance = std::numeric_limits<double>::max();

    // Loop through ecalPoints to find the closest recHit to our track
    for (const auto& ecal_rec_hit : ecal_points) {
      double temp_distance =
          calculateDistance(extrapolated_point, ecal_rec_hit);

      if (temp_distance < min_distance) {
        min_distance = temp_distance;
        closest_rec_hit = &ecal_rec_hit;
      }
    }  // for recHits

    if (closest_rec_hit != nullptr) {
      trk.setFirstLayerEcalRecHit(*closest_rec_hit);
      trk.setDistancetoRecHit(min_distance);
      trk.setEcalLayer1Location(extrapolated_point);
    }
  }  // make sure there are ecalPoints to work with

  return trk;
}

void LinearTruthTracking::onProcessEnd() {
  ldmx_log(info) << "AVG Time / Event: " << std::fixed << std::setprecision(1)
                 << processing_time_ / n_events_ << " ms";
  ldmx_log(info) << "Number of Events without enough seed points: "
                 << n_missing_;
  ldmx_log(info) << "Number of Events without trackID == 1 combo: " << n_empty_;
  ldmx_log(info) << "Total Truth Tracks / Event: " << n_truth_ << "/"
                 << n_events_;
}  // onProcessEnd

std::vector<ldmx::Measurement> LinearTruthTracking::findTruthHits(
    const std::vector<ldmx::Measurement>& hit_collection) {
  std::vector<ldmx::Measurement> layer1, layer2, layer3, layer4;

  // Split hits into layers based on z position
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

  // Construct combinations of hits on all layers until we find the combination
  // with 4 recoil e- hits (trackID = 1)
  // TODO: should we also allow combinations of 3 recoil e- hits in the truth
  // fitting?
  for (const auto& hit1 : layer1) {
    for (const auto& hit2 : layer2) {
      for (const auto& hit3 : layer3) {
        for (const auto& hit4 : layer4) {
          std::vector<ldmx::Measurement> combination = {hit1, hit2, hit3, hit4};
          auto truth_info = truth_matching_tool_->TruthMatch(combination);
          if (truth_info.trackID == 1.0 and truth_info.truthProb == 1.0) {
            return combination;
          }  // if trackID
        }    // for layer4
      }      // for layer3
    }        // for layer2
  }          // for layer1

  return {};  // if we can't find the recoil e-, don't return any points

}  // findTruthHits

double LinearTruthTracking::calculateDistance(
    const std::array<double, 3>& point1, const std::array<double, 3>& point2) {
  return sqrt(pow(point1[1] - point2[1], 2) + pow(point1[2] - point2[2], 2));
}  // calculateDistance

std::tuple<double, double, double, double, std::vector<double>>
LinearTruthTracking::fit3DLine(const std::vector<ldmx::Measurement>& points) {
  std::vector<double> z_vals, x_vals, y_vals;

  for (const auto& point : points) {
    const auto& position = point.getGlobalPosition();
    z_vals.push_back(position[0]);
    x_vals.push_back(position[1]);
    y_vals.push_back(position[2]);
  }  // get coordinates

  double sigma_x = recoil_truth_uncertainty_[0];

  // TODO: technically, the y uncertainty for a layer hit is the length of the
  // strip
  // TODO: for now, I assign the same uncertainty as a sensor (geometric
  // combination of two layers)
  double sigma_y = recoil_truth_uncertainty_[1];

  std::vector<double> weights(2 * points.size());

  for (size_t i = 0; i < points.size(); ++i) {
    weights[2 * i] = 1.0 / (sigma_x * sigma_x);
    weights[2 * i + 1] = 1.0 / (sigma_y * sigma_y);
  }  // construct weights matrix

  Eigen::MatrixXd A(2 * points.size(), 4);
  Eigen::VectorXd d(2 * points.size());
  Eigen::VectorXd w(2 * points.size());

  for (size_t i = 0; i < points.size(); ++i) {
    double z = z_vals[i], x = x_vals[i], y = y_vals[i];

    // Fill the A matrix (z, 1, 0, 0) for x and (0, 0, z, 1) for y
    A(2 * i, 0) = z;
    A(2 * i, 1) = 1;
    A(2 * i, 2) = 0;
    A(2 * i, 3) = 0;

    A(2 * i + 1, 0) = 0;
    A(2 * i + 1, 1) = 0;
    A(2 * i + 1, 2) = z;
    A(2 * i + 1, 3) = 1;

    // Fill the d vector with x and y values
    d(2 * i) = x;
    d(2 * i + 1) = y;

    // Fill the weights vector
    w(2 * i) = weights[2 * i];          // Weight for x
    w(2 * i + 1) = weights[2 * i + 1];  // Weight for y
  }

  // Solve the weighted least squares system
  Eigen::MatrixXd At_W_A = A.transpose() * w.asDiagonal() * A;
  Eigen::MatrixXd At_W_d = A.transpose() * w.asDiagonal() * d;
  Eigen::VectorXd m = At_W_A.ldlt().solve(At_W_d);

  Eigen::Matrix4d covariance_matrix = At_W_A.inverse();

  std::vector<double> covariance_vector = {
      covariance_matrix(0, 0), covariance_matrix(0, 1), covariance_matrix(0, 2),
      covariance_matrix(0, 3), covariance_matrix(1, 1), covariance_matrix(1, 2),
      covariance_matrix(1, 3), covariance_matrix(2, 2), covariance_matrix(2, 3),
      covariance_matrix(3, 3)};

  // Return results (mx, bx, my, by, covariance matrix as vector)
  return {m(0), m(1), m(2), m(3), covariance_vector};
}  // fit3Dline

double LinearTruthTracking::globalChiSquare(
    const std::vector<ldmx::Measurement>& points, double mx, double my,
    double bx, double by) {
  double chi2_x = 0, chi2_y = 0;

  for (const auto& point : points) {
    const auto& position = point.getGlobalPosition();
    double z = position[0];
    double x = position[1];
    double y = position[2];

    double x_fit = mx * z + bx;
    double y_fit = my * z + by;

    chi2_x += std::pow((x - x_fit) / recoil_truth_uncertainty_[0], 2);
    chi2_y += std::pow((y - y_fit) / recoil_truth_uncertainty_[1], 2);
  }

  return chi2_x + chi2_y;
}  // globalChiSquare

}  // namespace reco
}  // namespace tracking

DECLARE_PRODUCER_NS(tracking::reco, LinearTruthTracking);

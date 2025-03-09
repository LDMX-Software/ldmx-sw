#include "Tracking/Reco/LinearTruthTracking.h"

#include "Eigen/Dense"

namespace tracking {
namespace reco {

LinearTruthTracking::LinearTruthTracking(const std::string& name,
                                         framework::Process& process)
    : TrackingGeometryUser(name, process) {}

void LinearTruthTracking::configure(framework::config::Parameters& parameters) {
  // Output seed name
  out_trk_collection_ = parameters.getParameter<std::string>(
      "out_trk_collection", "LinearRecoilTruthTracks");

  // Input strip hits
  input_hits_collection_ = parameters.getParameter<std::string>(
      "input_hits_collection", "RecoilSimHits");
  input_rec_hits_collection_ = parameters.getParameter<std::string>(
      "input_recHits_collection", "EcalRecHits");

  input_pass_name_ =
      parameters.getParameter<std::string>("input_pass_name", "");

  ecal_first_layer_z_threshold_ =
      parameters.getParameter<double>("ecal_first_layer_z_threshold");
}  // configure

void LinearTruthTracking::produce(framework::Event& event) {
  auto start = std::chrono::high_resolution_clock::now();
  std::vector<ldmx::StraightTrack> straight_truth_tracks;
  n_events_++;

  const std::vector<ldmx::SimTrackerHit> recoil_hits =
      event.getCollection<ldmx::SimTrackerHit>(input_hits_collection_,
                                               input_pass_name_);
  const std::vector<ldmx::EcalHit> ecal_rec_hit =
      event.getCollection<ldmx::EcalHit>(input_rec_hits_collection_,
                                         input_pass_name_);

  std::vector<std::array<double, 3>> first_layer_ecal_rec_hits;

  for (const auto& x_ecal : ecal_rec_hit) {
    if (x_ecal.getZPos() < ecal_first_layer_z_threshold_) {
      first_layer_ecal_rec_hits.push_back(
          {x_ecal.getZPos(), x_ecal.getXPos(), x_ecal.getYPos()});
    }  // if first layer of Ecal
  }    // for positions in ecalRecHit

  // Check if we would fit empty seeds.
  if (recoil_hits.size() < 2) {
    n_missing_++;
    n_truth_ += straight_truth_tracks.size();
    event.add(out_trk_collection_, straight_truth_tracks);
    return;
  }  // if not enough hits

  std::vector<ldmx::SimTrackerHit> truth_points;

  // Find only recoil electron out of the hit collection
  for (const auto& point : recoil_hits) {
    if (point.getTrackID() == 1) {
      truth_points.push_back(point);
    }  // if trackID == 1
  }    // for point in hit collection

  // should only find 1 track (recoil e-) or 0 track (no recoil e-)
  if (truth_points.size() > 1) {
    straight_truth_tracks.push_back(
        truthTracker(truth_points, first_layer_ecal_rec_hits));
  }  // if we have truth points
  else {
    n_empty_++;
    n_truth_ += straight_truth_tracks.size();
    return;
  }  // else no truth points

  n_truth_ += straight_truth_tracks.size();
  event.add(out_trk_collection_, straight_truth_tracks);

  auto end = std::chrono::high_resolution_clock::now();
  auto diff = end - start;
  processing_time_ += std::chrono::duration<double, std::milli>(diff).count();

  first_layer_ecal_rec_hits.clear();
  straight_truth_tracks.clear();

}  // produce

ldmx::StraightTrack LinearTruthTracking::truthTracker(
    const std::vector<ldmx::SimTrackerHit>& points,
    std::vector<std::array<double, 3>>& ecal_points) {
  ldmx::StraightTrack trk = ldmx::StraightTrack();

  // We have pre-selected hits corresponding to the Recoil e-, so we only fit
  // these (no RecHit)
  auto [m_x, b_x, m_y, b_y] = fit3DLine(points);

  trk.setSlopeX(m_x);
  trk.setInterceptX(b_x);
  trk.setSlopeY(m_y);
  trk.setInterceptY(b_y);
  trk.setTheta(std::atan2(m_y, std::sqrt(1 + m_x * m_x)));
  trk.setPhi(std::atan2(m_x, 1.0));
  trk.setTargetLocation(0.0, b_x, b_y);
  trk.setNhits(points.size());
  trk.setNdf(points.size() - 2);

  trk.setTrackID(points[0].getTrackID());
  trk.setPdgID(points[0].getPdgID());
  trk.setTruthProb(1.0);

  trk.setAllTruthSensorPoints(points);

  trk.setChi2(globalChiSquare(points, m_x, m_y, b_x, b_y));

  // Z position from the first point in ecalPoints
  if (ecal_points.size() > 0) {
    double ecal_first_layer_z = ecal_points[0][0];

    // Extrapolate track to first layer of Ecal
    std::array<double, 3> extrapolated_point = {ecal_first_layer_z,
                                                m_x * ecal_first_layer_z + b_x,
                                                m_y * ecal_first_layer_z + b_y};

    const std::array<double, 3>* closest_rec_hit = nullptr;
    double min_distance = std::numeric_limits<double>::max();

    // Loop through ecalPoints to find the closest recHit to our track
    for (const auto& ecal_rec_hit : ecal_points) {
      double temp_distance =
          calculateDistance(extrapolated_point, ecal_rec_hit);

      if (temp_distance < min_distance) {
        min_distance = temp_distance;
        closest_rec_hit = &ecal_rec_hit;
      }  // if compare min distance
    }    // for recHits

    if (closest_rec_hit != nullptr) {
      trk.setFirstLayerEcalRecHit(*closest_rec_hit);
      trk.setDistancetoRecHit(min_distance);
      trk.setEcalLayer1Location(extrapolated_point);
    }  // if set trk info
  }    // make sure there are ecalPoints to work with

  return trk;
}  // truthTracker

void LinearTruthTracking::onProcessEnd() {
  ldmx_log(info) << "AVG Time / Event: " << std::fixed << std::setprecision(1)
                 << processing_time_ / n_events_ << " ms";
  ldmx_log(info) << "Number of Events without enough seed points: "
                 << n_missing_;
  ldmx_log(info) << "Number of Events with only 1 trackID == 1 point: "
                 << n_empty_;
  ldmx_log(info) << "Total Truth Tracks / Event: " << n_truth_ << "/"
                 << n_events_;
}  // onProcessEnd

double LinearTruthTracking::calculateDistance(
    const std::array<double, 3>& point1, const std::array<double, 3>& point2) {
  return sqrt(pow(point1[1] - point2[1], 2) + pow(point1[2] - point2[2], 2));
}  // calculateDistance

std::tuple<double, double, double, double> LinearTruthTracking::fit3DLine(
    const std::vector<ldmx::SimTrackerHit>& points) {
  std::vector<double> z_vals, x_vals, y_vals;

  for (const auto& point : points) {
    const auto& position = point.getPosition();
    x_vals.push_back(position[0]);
    y_vals.push_back(position[1]);
    z_vals.push_back(position[2]);
  }

  size_t num_points = points.size();
  Eigen::MatrixXd a_mat(num_points, 2);
  Eigen::VectorXd x_vec(num_points), y_vec(num_points);

  for (size_t i = 0; i < num_points; ++i) {
    a_mat(i, 0) = z_vals[i];
    a_mat(i, 1) = 1;
    x_vec(i) = x_vals[i];
    y_vec(i) = y_vals[i];
  }

  Eigen::Matrix2d AtA = a_mat.transpose() * a_mat;
  Eigen::Vector2d At_x = a_mat.transpose() * x_vec;
  Eigen::Vector2d At_y = a_mat.transpose() * y_vec;

  Eigen::Vector2d x_params = AtA.ldlt().solve(At_x);
  Eigen::Vector2d y_params = AtA.ldlt().solve(At_y);

  // Return (mx, bx, my, by)
  return {x_params(0), x_params(1), y_params(0), y_params(1)};
}

double LinearTruthTracking::globalChiSquare(
    const std::vector<ldmx::SimTrackerHit>& points, double m_x, double m_y,
    double b_x, double b_y) {
  double chi2_x = 0, chi2_y = 0;

  for (const auto& point : points) {
    const auto& position = point.getPosition();
    double x_pos = position[0];
    double y_pos = position[1];
    double z_pos = position[2];

    double x_fit = m_x * z_pos + b_x;
    double y_fit = m_y * z_pos + b_y;

    chi2_x += std::pow((x_pos - x_fit), 2);
    chi2_y += std::pow((y_pos - y_fit), 2);
  }

  return chi2_x + chi2_y;
}  // globalChiSquare

}  // namespace reco
}  // namespace tracking

DECLARE_PRODUCER_NS(tracking::reco, LinearTruthTracking);

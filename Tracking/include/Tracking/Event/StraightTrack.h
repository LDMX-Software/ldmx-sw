#ifndef TRACKING_EVENT_STRAIGHTTRACK_H_
#define TRACKING_EVENT_STRAIGHTTRACK_H_

//----------------------//
//   C++ Standard Lib   //
//----------------------//
#include <array>
#include <iostream>
#include <optional>
#include <vector>

//----------//
//   ROOT   //
//----------//
#include "Measurement.h"
#include "TObject.h"

namespace ldmx {

class StraightTrack {
 public:
  StraightTrack() = default;

  /**
   * Destructor.
   *
   * Currently, the destructor does nothing.
   */
  virtual ~StraightTrack() = default;

  /**
   * Print the string representation of this object.
   *
   * This class is needed by ROOT when building the dictionary.
   */
  void Print() const;

  // To match the Framework Bus clear. It's doing nothing
  void Clear(){};

  void setNhits(int nhits) { n_hits_ = nhits; }
  int getNhits() const { return n_hits_; }

  void setNdf(int ndf) { ndf_ = ndf; }
  int getNdf() const { return ndf_; }

  void setChi2(double chi2) { chi2_ = chi2; }
  double getChi2() const { return chi2_; }

  void setSlopeX(double slope_x) { slope_x_ = slope_x; }
  double getSlopeX() const { return slope_x_; }

  void setInterceptX(double intercept_x) { intercept_x_ = intercept_x; }
  double getInterceptX() const { return intercept_x_; }

  void setSlopeY(double slope_y) { slope_y_ = slope_y; }
  double getSlopeY() const { return slope_y_; }

  void setInterceptY(double intercept_y) { intercept_y_ = intercept_y; }
  double getInterceptY() const { return intercept_y_; }

  void setDistancetoRecHit(double distance) { distance_to_rec_hit_ = distance; }
  double getDistanceToRecHit() const { return distance_to_rec_hit_; }

  void setTheta(double theta) { theta_ = theta; }
  double getTheta() const { return theta_; }

  void setPhi(double phi) { phi_ = phi; }
  double getPhi() const { return phi_; }

  const std::vector<ldmx::Measurement>& getAllSensorPoints() const {
    return both_sensors_;
  }
  void setAllSensorPoints(const std::vector<ldmx::Measurement>& sensor_points) {
    both_sensors_ = sensor_points;
  }

  void setFirstSensorPosition(const std::array<double, 3>& first_sensor) {
    first_sensor_ = first_sensor;
  }
  std::array<double, 3> getFirstSensorPosition() const { return first_sensor_; }

  void setSecondSensorPosition(const std::array<double, 3>& second_sensor) {
    second_sensor_ = second_sensor;
  }
  std::array<double, 3> getSecondSensorPosition() const { return second_sensor_; }

  void setFirstLayerEcalRecHit(const std::array<double, 3>& ecal_hit) {
    ecal_rec_hit_ = ecal_hit;
  }
  std::array<double, 3> getFirstLayerEcalRecHit() const { return ecal_rec_hit_; }

  void setTargetLocation(const std::array<double, 3>& target_loc) {
    target_pos_ = target_loc;
  }
  void setEcalLayer1Location(const std::array<double, 3>& ecal_loc) {
    ecal_layer1_pos_ = ecal_loc;
  }

  void setTargetLocation(const double& z_pos, const double& x_pos, const double& y_pos) {
    target_pos_[0] = z_pos;
    target_pos_[1] = x_pos;
    target_pos_[2] = y_pos;
  }

  void setEcalLayer1Location(const double& z_pos, const double& x_pos, const double& y_pos) {
    ecal_layer1_pos_[0] = z_pos;
    ecal_layer1_pos_[1] = x_pos;
    ecal_layer1_pos_[2] = y_pos;
  }

  std::array<double, 3> getTargetLocation() const { return target_pos_; }
  double getTargetZ() const { return target_pos_[0]; }
  double getTargetX() const { return target_pos_[1]; }
  double getTargetY() const { return target_pos_[2]; }

  std::array<double, 3> getEcalLayer1Location() const { return ecal_layer1_pos_; }
  double getEcalLayer1Z() const { return ecal_layer1_pos_[0]; }
  double getEcalLayer1X() const { return ecal_layer1_pos_[1]; }
  double getEcalLayer1Y() const { return ecal_layer1_pos_[2]; }

  void setTrackID(int track_id) { track_id_ = track_id; }
  int getTrackID() const { return track_id_; }

  void setTruthProb(double truth_prob) { truth_prob_ = truth_prob; }
  double getTruthProb() const { return truth_prob_; }

  void setPdgID(int pdg_id) { pdg_id_ = pdg_id; }
  int getPdgID() const { return pdg_id_; }

  // Covariance vector has 10 elements arranged as a vector
  // mxmx mxbx mxmy mxby
  //      bxbx bxmy bxby
  //           mymy myby
  //                byby
  // m = slope, b = intercept
  void setCov(const std::vector<double>& cov) { trk_cov_ = cov; }
  std::vector<double> getCov() const { return trk_cov_; }

 protected:
  // Actual Track Parameters
  double slope_x_;
  double slope_y_;
  double intercept_x_;
  double intercept_y_;
  double distance_to_rec_hit_;
  double theta_;
  double phi_;

  std::vector<ldmx::Measurement> both_sensors_;
  std::array<double, 3> first_sensor_;
  std::array<double, 3> second_sensor_;
  std::array<double, 3> ecal_rec_hit_;

  int n_hits_;
  int ndf_;
  double chi2_;

  // The target location
  std::array<double, 3> target_pos_;
  // The ecal first layer position
  std::array<double, 3> ecal_layer1_pos_;

  // ID of the matched particle in the SimParticles map
  int track_id_{-1};
  // Truth probability
  double truth_prob_{0.};
  // pdgID (truth value)
  int pdg_id_{0};

  std::vector<double> trk_cov_;

  /// Class declaration needed by the ROOT dictionary.
  ClassDef(StraightTrack, 1);

};  // StraightTrack

typedef std::vector<ldmx::StraightTrack> StraightTracks;

}  // namespace ldmx

#endif  // TRACKING_EVENT_STRAIGHTTRACK_H_

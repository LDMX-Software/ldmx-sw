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
  int getNdf() const { return ndf_; };

  void setChi2(double chi2) { chi2_ = chi2; }
  double getChi2() const { return chi2_; }

  void setSlopeX(double slopeX) { slopeX_ = slopeX; }
  double getSlopeX() const { return slopeX_; }

  void setInterceptX(double interceptX) { interceptX_ = interceptX; }
  double getInterceptX() const { return interceptX_; }

  void setSlopeY(double slopeY) { slopeY_ = slopeY; }
  double getSlopeY() const { return slopeY_; }

  void setInterceptY(double interceptY) { interceptY_ = interceptY; }
  double getInterceptY() const { return interceptY_; }

  void setDistancetoRecHit(double distance) { distance_to_RecHit_ = distance; }
  double getDistanceToRecHit() const { return distance_to_RecHit_; }

  void setTheta(double theta) { theta_ = theta; }
  double getTheta() const { return theta_; }

  void setPhi(double phi) { phi_ = phi; }
  double getPhi() const { return phi_; }

  const std::vector<ldmx::Measurement>& getAllSensorPoints() const {
    return bothSensors_;
  };
  void setAllSensorPoints(const std::vector<ldmx::Measurement>& sensorPoints) {
    bothSensors_ = sensorPoints;
  }

  void setFirstSensorPosition(const std::array<double, 3>& firstSensor) {
    firstSensor_ = firstSensor;
  }
  std::array<double, 3> getFirstSensorPosition() const { return firstSensor_; };

  void setSecondSensorPosition(const std::array<double, 3>& secondSensor) {
    secondSensor_ = secondSensor;
  }
  std::array<double, 3> getSecondSensorPosition() const {
    return secondSensor_;
  };

  void setFirstLayerEcalRecHit(const std::array<double, 3>& ecal_hit) {
    ecalRecHit_ = ecal_hit;
  }
  std::array<double, 3> getFirstLayerEcalRecHit() const { return ecalRecHit_; };

  void setTargetLocation(const std::array<double, 3>& target_loc) {
    targetPos_ = target_loc;
  }
  void setEcalLayer1Location(const std::array<double, 3>& ecal_loc) {
    ecalLayer1Pos_ = ecal_loc;
  }

  void setTargetLocation(const double& z, const double& x, const double& y) {
    targetPos_[0] = z;
    targetPos_[1] = x;
    targetPos_[2] = y;
  }

  void setEcalLayer1Location(const double& z, const double& x,
                             const double& y) {
    ecalLayer1Pos_[0] = z;
    ecalLayer1Pos_[1] = x;
    ecalLayer1Pos_[2] = y;
  }

  std::array<double, 3> getTargetLocation() const { return targetPos_; };
  double getTargetZ() const { return targetPos_[0]; };
  double getTargetX() const { return targetPos_[1]; };
  double getTargetY() const { return targetPos_[2]; };

  std::array<double, 3> getEcalLayer1Location() const {
    return ecalLayer1Pos_;
  };
  double getEcalLayer1Z() const { return ecalLayer1Pos_[0]; };
  double getEcalLayer1X() const { return ecalLayer1Pos_[1]; };
  double getEcalLayer1Y() const { return ecalLayer1Pos_[2]; };

  void setTrackID(int trackid) { trackID_ = trackid; };
  int getTrackID() const { return trackID_; };

  void setTruthProb(double truthProb) { truthProb_ = truthProb; };
  double getTruthProb() const { return truthProb_; };

  void setPdgID(int pdgID) { pdgID_ = pdgID; };
  int getPdgID() const { return pdgID_; };

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
  double slopeX_;
  double slopeY_;
  double interceptX_;
  double interceptY_;
  double distance_to_RecHit_;
  double theta_;
  double phi_;

  std::vector<ldmx::Measurement> bothSensors_;
  std::array<double, 3> firstSensor_;
  std::array<double, 3> secondSensor_;
  std::array<double, 3> ecalRecHit_;

  int n_hits_;
  int ndf_;
  double chi2_;

  // The target location
  std::array<double, 3> targetPos_;
  // The ecal first layer position
  std::array<double, 3> ecalLayer1Pos_;

  // ID of the matched particle in the SimParticles map
  int trackID_{-1};
  // Truth probability
  double truthProb_{0.};
  // pdgID (truth value)
  int pdgID_{0};

  std::vector<double> trk_cov_;

  /// Class declaration needed by the ROOT dictionary.
  ClassDef(StraightTrack, 1);

};  // StraightTrack

typedef std::vector<ldmx::StraightTrack> StraightTracks;

}  // namespace ldmx

#endif  // TRACKING_EVENT_STRAIGHTTRACK_H_

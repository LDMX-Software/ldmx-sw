#ifndef TRACKING_EVENT_TRACK_H_
#define TRACKING_EVENT_TRACK_H_

//----------------------//
//   C++ Standard Lib   //
//----------------------//
#include <iostream>
#include <optional>
#include <vector>

//----------//
//   ROOT   //
//----------//
#include "TObject.h"

// --- ACTS --- //
//#include "Acts/Definitions/TrackParametrization.hpp"
//#include "Acts/EventData/TrackParameters.hpp"

namespace ldmx {

/// This enum describes the type of TrackState
/// RefPoint is wrt to a line parallel to the Z axis located at the refPoint
/// stored in the TrackState AtTarget is wrt the target surface: i.e. a surface
/// at the refPoint with orientation as the ACTS Tracking Frame
/// AtFirstMeasurement: track state at the first measurment on track.
/// For the recoil "first" means closest to the target, for the tagger it means
/// farthest from the target

/// AtLastMeasurement : track state at the last measurement on track.
/// For the recoil it means closest to the ECAL, for the tagger closest to the
/// target.

enum TrackStateType {
  RefPoint = 0,
  AtTarget = 1,
  AtFirstMeasurement = 2,
  AtLastMeasurement = 3,
  AtECAL = 4,
  AtBeamOrigin = 5,
  Invalid = 6
};

/**
 * Implementation of a track object.
 *
 * This class encapsulates all the information of a particle trajectory in the
 * tracker
 *
 */

class Track {
 public:
  // Track states won't be visualized in the root tree from the TBrowser, but it
  // will be accessible when reading back the rootfile using for example the
  // monitoring code.
  struct TrackState {
    double refX, refY, refZ;
    std::vector<double> params;
    std::vector<double> cov;
    TrackStateType ts_type;
  };

  Track(){};

  /**
   * Destructor.
   *
   * Currently, the destructor does nothing.
   */
  virtual ~Track(){};

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

  std::optional<TrackState> getTrackState(TrackStateType tstype) const {
    for (auto ts : trackStates_)
      if (ts.ts_type == tstype) return std::optional<TrackState>(ts);

    return std::nullopt;
  }

  // void setNholes(int nholes) {n_holes_ = nholes;}
  // int  getNholes() const {return n_holes_;}

  void setNoutliers(int nout) { n_outliers_ = nout; }
  int getNoutliers() const { return n_outliers_; }

  void setNdf(int ndf) { ndf_ = ndf; }
  int getNdf() const { return ndf_; };

  void setNsharedHits(int nsh) { n_shared_hits_ = nsh; }
  int getNsharedHits() const { return n_shared_hits_; }

  void setChi2(double chi2) { chi2_ = chi2; }
  double getChi2() const { return chi2_; }

  void setTrackID(int trackid) { trackID_ = trackid; };
  int getTrackID() const { return trackID_; };

  void setTruthProb(double truthProb) { truthProb_ = truthProb; };
  double getTruthProb() const { return truthProb_; };

  void setPdgID(int pdgID) { pdgID_ = pdgID; };
  int getPdgID() const { return pdgID_; };

  // in units of e
  int q() const { return perigee_pars_[4] > 0 ? 1 : -1; }

  // Add measurement indices to tracks
  // For reco  tracks they corresponds to the indices in the measurement
  // container For truth tracks they corresponds to the indices of the
  // SimHitCointainer

  void addMeasurementIndex(unsigned int measIdx) {
    meas_idxs_.push_back(measIdx);
  }
  std::vector<unsigned int> getMeasurementsIdxs() const { return meas_idxs_; }

  /// d_0 z_0 phi_0 theta q/p t
  // void setPerigeeParameters(const Acts::BoundVector& par)  {perigee_pars_ =
  // par; } Acts::BoundVector getPerigeeParameters() {return perigee_pars_;}

  // void setPerigeeCov(const Acts::BoundMatrix& cov) {perigee_cov_ = cov;}
  // Acts::BoundMatrix getPerigeeCov() {return perigee_cov_;}

  // void setPerigeeState(const Acts::BoundVector& par, const Acts::BoundMatrix&
  // cov) {
  //   perigee_pars_ = par;
  //   perigee_cov_  = cov;
  // }

  // Vector representation
  void setPerigeeParameters(const std::vector<double>& par) {
    perigee_pars_ = par;
  }
  std::vector<double> getPerigeeParameters() const { return perigee_pars_; }

  void setPerigeeCov(const std::vector<double>& cov) { perigee_cov_ = cov; }
  std::vector<double> getPerigeeCov() const { return perigee_cov_; }

  void setPerigeeLocation(const std::vector<double>& perigee) {
    perigee_ = perigee;
  }

  void setPerigeeLocation(const double& x, const double& y, const double& z) {
    perigee_[0] = x;
    perigee_[1] = y;
    perigee_[2] = z;
  }

  void setMomentum(const double& px, const double& py, const double& pz) {
    momentum_[0] = px;
    momentum_[1] = py;
    momentum_[2] = pz;
  }

  void setPosition(const double& x, const double& y, const double& z) {
    position_[0] = x;
    position_[1] = y;
    position_[2] = z;
  }

  std::vector<double> getPerigeeLocation() const { return perigee_; };
  double getPerigeeX() const { return perigee_[0]; };
  double getPerigeeY() const { return perigee_[1]; };
  double getPerigeeZ() const { return perigee_[2]; };

  std::vector<double> getMomentum() const { return momentum_; };
  std::vector<double> getPosition() const { return position_; };

  // getters -- TODO use an enum instead

  double getD0() const { return perigee_pars_[0]; };
  double getZ0() const { return perigee_pars_[1]; };
  double getPhi() const { return perigee_pars_[2]; };
  double getTheta() const { return perigee_pars_[3]; };
  double getQoP() const { return perigee_pars_[4]; };
  double getT() const { return perigee_pars_[5]; };

  void addTrackState(const ldmx::Track::TrackState& ts) {
    trackStates_.push_back(ts);
  };

  std::vector<TrackState> getTrackStates() const { return trackStates_; }

 protected:
  int n_hits_{0};
  int n_outliers_{0};
  int ndf_{0};
  int n_shared_hits_{0};
  int n_holes_{0};

  // particle hypothesis if truth track
  // int pdgID_{0};

  double chi2_{0};

  // The parameters and covariance matrix wrt the perigee surface
  // Acts::BoundVector perigee_pars_;
  // Acts::BoundSymMatrix perigee_cov_;

  // 6 elements
  // d0 / z0 / phi / theta / qop / t
  std::vector<double> perigee_pars_{0., 0., 0., 0., 0., 0.};

  // 21 elements
  // d0d0 d0z0 d0phi d0th  d0qop  d0t
  //      z0z0 z0phi z0th  z0qop  z0t
  //           phph  phith phqop  pht
  //                  thth thqop  tht
  //                       qopqop qopt
  //                              t
  std::vector<double> perigee_cov_;

  // The perigee location
  std::vector<double> perigee_{0., 0., 0.};

  // The 3-momentum at the perigee
  std::vector<double> momentum_{0., 0., 0.};

  // The 3-position at the perigee
  std::vector<double> position_{0., 0., 0.};

  // The vector of measurement IDs
  std::vector<unsigned int> meas_idxs_{};

  // ID of the matched particle in the SimParticles map
  int trackID_{-1};

  // Truth probability
  double truthProb_{0.};

  // pdgID
  int pdgID_{0};

  // Track States
  std::vector<TrackState> trackStates_;

  /// Class declaration needed by the ROOT dictionary.
  ClassDef(Track, 2);

};  // Track

typedef std::vector<ldmx::Track> Tracks;
// typedef std::vector<std::reference_wrapper<const ldmx::Track>> Tracks;

}  // namespace ldmx

#endif  // TRACKING_EVENT_TRACK_H_

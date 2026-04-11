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
// #include "Acts/Definitions/TrackParametrization.hpp"
// #include "Acts/EventData/TrackParameters.hpp"

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
  Invalid = 6,
  AtHCAL = 7
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
  // All positions, momenta, and covariances are in the LDMX global frame:
  //   x: horizontal (bend plane), y: vertical, z: downstream beam direction.
  // ACTS uses a rotated frame (x downstream, y horizontal, z vertical).
  // The conversion ACTS->LDMX is: x_ldmx=y_acts, y_ldmx=z_acts, z_ldmx=x_acts.
  // This rotation must be applied before filling these fields (see
  // makeTrackState in TrackingUtils.h).
  struct TrackState {
    std::vector<double> pos_{-666., -666.,
                             -666.};  // (x, y, z) in mm, LDMX global
    std::vector<double> mom_{-666., -666.,
                             -666.};  // (px, py, pz) in MeV, LDMX global
    // 21-element upper-triangular 6x6 covariance over (x, y, z, px, py, pz)
    // in LDMX global coordinates, units: mm^2 (pos-pos), mm*MeV (pos-mom),
    // MeV^2 (mom-mom): xx xy xz xpx xpy xpz
    //    yy yz ypx ypy ypz
    //       zz zpx zpy zpz
    //          pxpx pxpy pxpz
    //               pypy pypz
    //                    pzpz
    std::vector<double> pos_mom_cov_;
    TrackStateType ts_type_;
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
  friend std::ostream& operator<<(std::ostream& o, const Track& d);

  // To match the Framework Bus clear. It's doing nothing
  void clear() {};

  void setNhits(int nhits) { n_hits_ = nhits; }
  int getNhits() const { return n_hits_; }

  std::optional<TrackState> getTrackState(TrackStateType tstype) const {
    for (auto ts : track_states_)
      if (ts.ts_type_ == tstype) return std::optional<TrackState>(ts);

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

  void setTrackID(int trackid) { track_id_ = trackid; };
  int getTrackID() const { return track_id_; };

  void setTruthProb(double truthProb) { truth_prob_ = truthProb; };
  double getTruthProb() const { return truth_prob_; };

  void setPdgID(int pdgID) { pdg_id_ = pdgID; };
  int getPdgID() const { return pdg_id_; };

  // Add measurement indices to tracks
  // For reco  tracks they corresponds to the indices in the measurement
  // container For truth tracks they corresponds to the indices of the
  // SimHitCointainer

  void addDedxMeasurement(float path_length) {
    dedx_measurements_.push_back(path_length);
  }
  std::vector<float> getDedxMeasurements() const { return dedx_measurements_; }

  void addMeasurementIndex(unsigned int measIdx) {
    meas_idxs_.push_back(measIdx);
  }
  std::vector<unsigned int> getMeasurementsIdxs() const { return meas_idxs_; }

  void addOutlierIndex(unsigned int measIdx) {
    outlier_idxs_.push_back(measIdx);
  }
  std::vector<unsigned int> getOutlierIdxs() const { return outlier_idxs_; }

  void addHoleIndex(unsigned int measIdx) { hole_idxs_.push_back(measIdx); }
  std::vector<unsigned int> getHoleIdxs() const { return hole_idxs_; }

  void addSharedIndex(unsigned int measIdx) { shared_idxs_.push_back(measIdx); }
  std::vector<unsigned int> getSharedIdxs() const { return shared_idxs_; }

  // Per-hit smoothed state at each measurement surface (same order as
  // meas_idxs_).  Used to form unbiased residuals via the algebraic
  // leave-one-out formula of NIM A 262, 444 (1987):
  //   r_ubs  = V/(V - C) * (m - x_smooth)
  //   pull   = (m - x_smooth) / sqrt(V - C)
  // where V = cov_uu, C = smoothed_cov_loc0.
  void addSmoothedLoc0(float loc0, float cov_loc0) {
    smoothed_loc0_.push_back(loc0);
    smoothed_cov_loc0_.push_back(cov_loc0);
  }
  const std::vector<float>& getSmoothedLoc0() const { return smoothed_loc0_; }
  const std::vector<float>& getSmoothedCovLoc0() const {
    return smoothed_cov_loc0_;
  }

  void setCharge(int q) { charge_ = q; }
  double getCharge() const { return charge_; }

  void setTime(double time) { time_ = time; }
  double getTime() const { return time_; }

  // Perigee parameters at the target surface: d0, z0, phi, theta, q/p, t
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
  std::vector<double> getPerigeeLocation() const { return perigee_; }
  double getPerigeeX() const { return perigee_[0]; }
  double getPerigeeY() const { return perigee_[1]; }
  double getPerigeeZ() const { return perigee_[2]; }

  double getD0() const { return perigee_pars_[0]; }
  double getZ0() const { return perigee_pars_[1]; }
  double getPhi() const { return perigee_pars_[2]; }
  double getTheta() const { return perigee_pars_[3]; }
  double getQoP() const { return perigee_pars_[4]; }
  double getT() const { return perigee_pars_[5]; }

  void addTrackState(const ldmx::Track::TrackState& ts) {
    track_states_.push_back(ts);
  };

  std::vector<TrackState> getTrackStates() const { return track_states_; }

  /// Returns the momentum (px, py, pz) in MeV in the LDMX global frame
  /// from the AtTarget TrackState. Returns an empty vector if not available.
  std::vector<double> getMomentumAtTarget() const {
    for (const auto& ts : track_states_)
      if (ts.ts_type_ == AtTarget) return ts.mom_;
    return {};
  }

  /// Returns the position (x, y, z) in mm in the LDMX global frame
  /// from the AtTarget TrackState. Returns an empty vector if not available.
  std::vector<double> getPositionAtTarget() const {
    for (const auto& ts : track_states_)
      if (ts.ts_type_ == AtTarget) return ts.pos_;
    return {};
  }

  /// Returns the momentum (px, py, pz) in MeV in the LDMX global frame
  /// for the requested TrackState type. Returns an empty vector if not
  /// available.
  std::vector<double> getMomentum(TrackStateType tstype) const {
    for (const auto& ts : track_states_)
      if (ts.ts_type_ == tstype) return ts.mom_;
    return {};
  }

  /// Returns the position (x, y, z) in mm in the LDMX global frame
  /// for the requested TrackState type. Returns an empty vector if not
  /// available.
  std::vector<double> getPosition(TrackStateType tstype) const {
    for (const auto& ts : track_states_)
      if (ts.ts_type_ == tstype) return ts.pos_;
    return {};
  }

  /// Returns the 21-element upper-triangular covariance vector over
  /// (x, y, z, px, py, pz) in LDMX global coordinates for the requested
  /// TrackState type. Units: mm^2 (pos-pos), mm*MeV (pos-mom), MeV^2 (mom-mom).
  /// Returns an empty vector if not available.
  std::vector<double> getCovariance(TrackStateType tstype) const {
    for (const auto& ts : track_states_)
      if (ts.ts_type_ == tstype) return ts.pos_mom_cov_;
    return {};
  }

 protected:
  int n_hits_{0};
  int n_outliers_{0};
  int ndf_{0};
  int n_shared_hits_{0};
  int n_holes_{0};

  double chi2_{0};

  // Perigee parameters at the target surface, expressed in LDMX global frame.
  // Parameter order: d0 / z0 / phi / theta / qop / t
  // d0d0 d0z0 d0phi d0th  d0qop  d0t
  //      z0z0 z0phi z0th  z0qop  z0t
  //           phph  phith phqop  pht
  //                  thth thqop  tht
  //                       qopqop qopt
  //                              tt
  std::vector<double> perigee_pars_{0., 0., 0., 0., 0., 0.};
  std::vector<double> perigee_cov_;
  std::vector<double> perigee_{0., 0.,
                               0.};  // perigee location in mm, LDMX global

  // dE/dx measurements (path length in MeV/mm)
  std::vector<float> dedx_measurements_{};

  // The vector of measurement IDs
  std::vector<unsigned int> meas_idxs_{};

  // The vector of outlier IDs
  std::vector<unsigned int> outlier_idxs_{};

  // The vector of hole IDs
  std::vector<unsigned int> hole_idxs_{};

  // The vector of shared hit IDs
  std::vector<unsigned int> shared_idxs_{};

  // Per-hit Kalman smoothed loc0 (mm) and its variance (mm²) at each
  // measurement surface, in the same order as meas_idxs_.
  std::vector<float> smoothed_loc0_{};
  std::vector<float> smoothed_cov_loc0_{};

  // ID of the matched particle in the SimParticles map
  int track_id_{-1};

  // Truth probability
  double truth_prob_{0.};

  // pdgID
  int pdg_id_{0};

  // Track charge
  int charge_{0};

  // Track time
  double time_{-666.0};

  // Track States
  std::vector<TrackState> track_states_;

  /// Class declaration needed by the ROOT dictionary.
  ClassDef(Track, 6);

};  // Track

typedef std::vector<ldmx::Track> Tracks;
// typedef std::vector<std::reference_wrapper<const ldmx::Track>> Tracks;

}  // namespace ldmx

#endif  // TRACKING_EVENT_TRACK_H_

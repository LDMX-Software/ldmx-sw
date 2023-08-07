#ifndef SIMCORE_USERTRACKINFORMATION_H
#define SIMCORE_USERTRACKINFORMATION_H

#include "G4ThreeVector.hh"
#include "G4Track.hh"
#include "G4VUserTrackInformation.hh"

namespace simcore {

/**
 * Provides user defined information to associate with a Geant4 track.
 *
 * This is helpful for keeping track of information we care about
 * that Geant4 doesn't persist by default.
 */
class UserTrackInformation : public G4VUserTrackInformation {
 public:
  /// Constructor
  UserTrackInformation() = default;

  virtual ~UserTrackInformation() = default;
  /**
   * get
   *
   * A static helper function for getting the track information
   * from the passed G4Track. If the track doesn't have an
   * information attached, a new one is created.
   *
   * @note The return value of this pointer is never NULL.
   *
   * @param[in] track G4Track to get information from
   */
  static UserTrackInformation* get(const G4Track* track);

  /**
   * Initialize the track information with the passed track.
   *
   * We assume the passed track is newly created
   * so we can copy its "current" kinematics and define
   * those kinematics to be the "vertex" kinematics.
   *
   * Even though we are "initializing" the track,
   * we only change the kinematic values. The boolean
   * flags may have been edited prior to the track reaching
   * its own processing phase (where it is initialized),
   * so those flags should (and are) not changed here.
   */
  void initialize(const G4Track* track);

  /// Print the information associated with the track.
  void Print() const final override;

  /**
   * Get the flag which indicates whether this track should be saved
   * as a Trajectory.
   *
   * @return The save flag.
   */
  bool getSaveFlag() const { return saveFlag_; }

  /**
   * Set the save flag so the associated track will be persisted
   * as a Trajectory.
   *
   * @param[in] saveFlag True to save the associated track.
   */
  void setSaveFlag(bool saveFlag) { saveFlag_ = saveFlag; }

  /**
   * Check whether this track is a brem candidate.
   *
   * @return True if this track is a brem candidate, false otherwise.
   */
  bool isBremCandidate() const { return isBremCandidate_; }

  /**
   * Tag this track as a brem candidate by the biasing filters.
   *
   * @param[in] isBremCandidate flag indicating whether this track is
   *      a candidate or not.
   */
  void tagBremCandidate(bool isBremCandidate = true) {
    isBremCandidate_ = isBremCandidate;
  }

  /**
   * Check whether this track is a photon that has undergone a
   * photo-nuclear reaction.
   *
   * @return True if this track is a photon that has undergone a
   * photo-nuclear reaction, false otherwise.
   */
  bool isPNGamma() const { return isPNGamma_; }

  /**
   * Tag this track as a photon that has undergone a photo-nuclear
   * reaction.
   *
   * @param[in] isPNGamma flag indicating whether this track has
   *      undergone a photo-nuclear reaction or not.
   */
  void tagPNGamma(bool isPNGamma = true) { isPNGamma_ = isPNGamma; }

  /**
   * Get the initial momentum 3-vector of the track [MeV].
   *
   * @return The initial momentum of the track.
   */
  const G4ThreeVector& getInitialMomentum() const { return initialMomentum_; }

  /**
   * Get the name of the volume that this track was created in.
   */
  std::string getVertexVolume() const { return vertexVolume_; }

  /**
   * Get the global time at which this track was created.
   */
  double getVertexTime() const { return vertex_time_; }

 private:
  /**
   * Flag for saving the track as a Trajectory.
   *
   * Default value is false because we want to save space
   * in the output file. We assume everywhere else that
   * the save flag is false unless some other part changes it.
   */
  bool saveFlag_{false};

  /// Flag indicating whether this track is a brem candidate
  bool isBremCandidate_{false};

  /**
   * Flag indicating whether this track has undergone a photo-nuclear
   * reaction.
   */
  bool isPNGamma_{false};

  /// Volume the track was created in.
  std::string vertexVolume_{""};

  /// Global Time of Creation
  double vertex_time_{0.};

  /// The initial momentum of the track.
  G4ThreeVector initialMomentum_;
};
}  // namespace simcore

#endif

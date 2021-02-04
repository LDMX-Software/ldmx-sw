/**
 * @file Trajectory.h
 * @brief Class providing Trajectory implementation for storing track info
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef SIMCORE_TRAJECTORY_H_
#define SIMCORE_TRAJECTORY_H_

// Geant4
#include "G4Allocator.hh"
#include "G4Track.hh"
#include "G4TrajectoryContainer.hh"
#include "G4VTrajectory.hh"

// STL
#include <cmath>
#include <vector>

typedef std::vector<G4VTrajectoryPoint*> TrajectoryPointContainer;

namespace simcore {

/**
 * @class Trajectory
 * @brief Trajectory implementation for storing track info for persistence and
 * visualization
 *
 * @note
 * Class is based on this Geant4 tip:
 * <a href="http://geant4.slac.stanford.edu/Tips/event/3.html">Trajectory Event
 * Tip</a>
 */
class Trajectory : public G4VTrajectory {
 public:
  /** Map of track ID to Trajectory objects. */
  typedef std::map<int, Trajectory*> TrajectoryMap;

  /**
   * Class constructor.
   * @param aTrack The Track from which to construct the trajectory.
   */
  Trajectory(const G4Track* aTrack);

  /**
   * Class destructor.
   */
  virtual ~Trajectory();

  /**
   * Create a new trajectory.
   * @param s The size of the object.
   */
  inline void* operator new(size_t s);

  /**
   * Delete a trajectory.
   * @param obj The object to delete.
   */
  inline void operator delete(void* obj);

  /**
   * Get the associated track ID.
   * @return The track ID.
   */
  G4int GetTrackID() const;

  /**
   * Get the parent track ID.
   * @return The parent track ID.
   */
  G4int GetParentID() const;

  /**
   * Get the name of the associated particle.
   * @return The name of the particle.
   */
  G4String GetParticleName() const;

  /**
   * Get the charge of the particle.
   * @return The charge of the particle.
   */
  G4double GetCharge() const;

  /**
   * Get the PDG encoding of the particle.
   * @return The PDG encoding of the particle.
   */
  G4int GetPDGEncoding() const;

  /**
   * Get the initial track momentum [MeV].
   * @return The initial track momentum.
   */
  G4ThreeVector GetInitialMomentum() const;

  /**
   * Get the number of trajectory points.
   * @return The number of trajectory points.
   */
  int GetPointEntries() const;

  /**
   * Get a trajectory point.
   * @param i The index of the trajectory point.
   */
  G4VTrajectoryPoint* GetPoint(G4int i) const;

  /**
   * Append a step to the trajectory.
   * @param aStep The step info.
   */
  void AppendStep(const G4Step* aStep);

  /**
   * Merge a trajectory into this one.
   * @param secondTrajectory The trajectory to merge into this one.
   */
  void MergeTrajectory(G4VTrajectory* secondTrajectory);

  /**
   * Get the trajectory end point [mm].
   * @return The trajectory end point.
   */
  G4ThreeVector getEndPoint() const;

  /**
   * Get the particle's energy [MeV].
   * @return The trajectory's energy.
   */
  G4double getEnergy() const;

  /**
   * Get the particle's mass [MeV].
   * @return The particle's mass.
   */
  G4double getMass() const;

  /**
   * Get the global creation time of the trajectory.
   *
   */
  G4float getGlobalTime() const;

  /**
   * Get the generator status (greater than zero for generator particle).
   * @return The particle's generator status.
   */
  G4int getGenStatus() const;

  /**
   * Get the particle's vertex position.
   * @return The particle's vertex position.
   */
  const G4ThreeVector& getVertexPosition() const;

  /**
   * Set the generator status of the particle.
   * @param genStatus The particle's generator status.
   */
  void setGenStatus(int genStatus);

  /**
   * Find a Trajectory by its track ID within a G4TrajectoryContainer.
   * @param trajCont The G4TrajectoryContainer to search.
   * @param trackID The track ID.
   * @return The matching Trajectory or null if does not exist.
   */
  static Trajectory* findByTrackID(G4TrajectoryContainer* trajCont,
                                   int trackID);

  /**
   * Get the creator process type of this particle.
   * This corresponds to the value returned by
   * <i>G4VProcess::GetProcessSubType()</i> e.g. 121 for products of
   * photonuclear reactions.
   * @return The creator process type of this particle.
   */
  int getProcessType() { return processType_; }

  void setEndPointMomentum(const G4Track* aTrack) {
    G4ThreeVector p = aTrack->GetMomentum();
    double px = p.x();
    double py = p.y();
    double pz = p.z();
    if (px == -0) px = 0;
    if (py == -0) py = 0;
    if (pz == -0) pz = 0;
    endPointMomentum_.set(px, py, pz);
  }

  const G4ThreeVector& getEndPointMomentum() const { return endPointMomentum_; }

  std::string getVertexVolume() const { return vertexVolume_; }
  std::string getVertexRegion() const { return vertexRegion_; }

 private:
  /** The list of trajectory points. */
  TrajectoryPointContainer* trajPoints_;

  /** The particle definition. */
  G4ParticleDefinition* particleDef_;

  /** The track ID. */
  G4int trackID_;

  /** The parent track ID. */
  G4int parentID_;

  /** The particle's energy. */
  G4double energy_;

  /** The particle's mass. */
  G4double mass_;

  /** The particle's global time. */
  G4float globalTime_;

  /** The particle's generator status. */
  G4int genStatus_;

  /** The particle's initial momentum. */
  G4ThreeVector initialMomentum_;

  /** The momentum at the particle's end point. */
  G4ThreeVector endPointMomentum_;

  /** The particle's vertex position. */
  G4ThreeVector vertexPosition_;

  /// The particle's creation volume
  std::string vertexVolume_;

  /**
   * The particle's create region
   * This variable is not stored in the output SimParticle event,
   * but it is helpful for the TrackMap::findIncident function.
   */
  std::string vertexRegion_;

  /** The type of the process which created the track. */
  int processType_;
};

/**
 * Custom memory allocator.
 */
extern G4Allocator<Trajectory> TrajectoryAllocator;

inline void* Trajectory::operator new(size_t) {
  void* aTrajectory;
  aTrajectory = (void*)TrajectoryAllocator.MallocSingle();
  return aTrajectory;
}

inline void Trajectory::operator delete(void* aTrajectory) {
  TrajectoryAllocator.FreeSingle((Trajectory*)aTrajectory);
}

}  // namespace simcore

#endif

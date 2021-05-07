/**
 * @file TrackMap.h
 * @brief Class which defines a map of track ID to parent ID and Trajectory
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef SIMCORE_TRACKMAP_H_
#define SIMCORE_TRACKMAP_H_

// STL
#include <unordered_map>

// Geant4
#include "G4Event.hh"
#include "G4Track.hh"

// LDMX
#include "SimCore/Event/SimParticle.h"
#include "SimCore/UserTrackInformation.h"
#include "SimCore/UserPrimaryParticleInformation.h"

namespace simcore {

/**
 * @class TrackMap
 * @brief Defines a map of particle ancestry and particles to be saved
 *
 * This class keeps track of the ancestry (child -> parent)
 * and descendents (parent -> children) of ALL particles generated
 * in an event. This allows the particles that are chosen to
 * be saved (via the TrackMap::save method) to have their
 * parent and children faithfully recorded in the output file.
 */
class TrackMap {
 public:
  /**
   * Add a record in the map connecting a track ID to its parent ID.
   * @param trackID The track ID.
   * @param parentID The parent track ID.
   */
  inline void insert(int trackID, int parentID) {
    ancestry_[trackID] = parentID;
    descendents_[parentID].push_back(trackID);
  }

  /**
   * Check if the passed track ID has already been inserted
   * into the track map.
   */
  inline bool contains(int trackID) const {
    ancestry_.find(trackID) == ancestry_.end();
  }

  /**
   * Find a trajectory's nearest parent that is incident on the calorimeter
   * region
   *
   * If this track ID does not have such a trajectory, then the
   * track ID of the primary in its parentage is returned.
   *
   * @param trackID The track ID of the trajectory to search its parentage for
   * the incident
   */
  int findIncident(int trackID) const;

  /**
   * Return true if the given track ID  is saved
   * i.e. will be stored in output file
   *
   * @param trackID The track ID.
   * @return True if the track ID has been inserted in output particle map
   * @note This method does <b>not</b> search through the track parentage for
   * the first available Trajectory.
   */
  inline bool isSaved(int trackID) const {
    return particle_map_.find(trackID) != particle_map_.end();
  }

  /**
   * Add a track to be stored into output map
   * @note We assume that the track is at the end of processing
   * so that its current kinematics can be labeled as the "end-point"
   * kinematics.
   * @param track G4Track to store into output
   */
  void save(const G4Track* track);

  /**
   * Trace the ancestry for the particles that will be stored.
   * This should be done at the end of the event before writing
   * the particle map to the event bus and involves looping
   * through the particles that will be saved.
   */
  void traceAncestry();

  /**
   * Clear the internal maps.
   *
   * This should be called at the **beginning** of an event.
   * The maps need to persist through the end of the event so
   * that they are available to be written to the output file.
   */
  void clear();

  /**
   * Get the map of particles to be stored in output event.
   */
  std::map<int,ldmx::SimParticle> &getParticleMap() {
    return particle_map_;
  }

 private:
  /// ancestry map of particles in event (child -> parent)
  std::unordered_map<int,int> ancestry_;

  /// descendents map of particles in event (parent -> children)
  std::unordered_map<int,std::vector<int>> descendents_;

  /// map of SimParticles that will be stored
  std::map<int,ldmx::SimParticle> particle_map_;
};

}  // namespace simcore

#endif

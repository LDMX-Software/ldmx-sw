/**
 * @file TrackMap.h
 * @brief Class which defines a map of track ID to parent ID and Trajectory
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef SIMCORE_TRACKMAP_H_
#define SIMCORE_TRACKMAP_H_

// Geant4
#include "G4Event.hh"
#include "G4VTrajectory.hh"

#include "SimCore/Trajectory.h"

namespace simcore {

/**
 * @class TrackMap
 * @brief Defines a map of track ID to parent ID and Trajectory
 *
 * @note
 * This class provides a record of track ancestry which is used
 * to connect track IDs to their parents.  It also maps track IDs
 * to Trajectory objects.
 */
class TrackMap {
 public:
  /**
   * Map of track ID to parent ID.
   */
  typedef std::map<G4int, G4int> TrackIDMap;

  /**
   * Add a record in the map connecting a track ID to its parent ID.
   * @param trackID The track ID.
   * @param parentID The parent track ID.
   */
  inline void addSecondary(G4int trackID, G4int parentID) {
    trackIDMap_[trackID] = parentID;
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
  int findIncident(G4int trackID);

  /**
   * Return true if the given track ID has an explicitly assigned trajectory.
   * @param trackID The track ID.
   * @return True if the track ID has an assigned Trajectory.
   * @note This method does <b>not</b> search through the track parentage for
   * the first available Trajectory.
   */
  inline bool hasTrajectory(G4int trackID) const {
    return trajectoryMap_.find(trackID) != trajectoryMap_.end();
  }

  /**
   * Add a Trajectory which will be associated with its track ID in the map.
   * @param traj The Trajectory to add.
   */
  inline void addTrajectory(Trajectory* traj) {
    trajectoryMap_[traj->GetTrackID()] = traj;
  }

  /**
   * Return true if the track ID is in the ancestry map
   * @return True if the track ID is in the ancestry map
   */
  bool contains(G4int trackID) const {
    return trackIDMap_.find(trackID) != trackIDMap_.end();
  }

  /**
   * Get parent track id
   * @param[in] track Id to get parent for
   * @return track ID of parent track
   */
  int getParent(int trackID) const { return trackIDMap_.at(trackID); }

  /**
   * Get a Trajectory from a track ID.
   * @return A Trajectory from a track ID.
   * @note Does not search for a parent Trajectory if this
   * track ID is not assigned to a Trajectory.
   */
  inline Trajectory* getTrajectory(G4int trackID) const {
    if (hasTrajectory(trackID)) {
      return trajectoryMap_.at(trackID);
    } else {
      return nullptr;
    }
  }

  /**
   * Clear the internal maps.
   */
  void clear();

 private:
  /** Map of track IDs to parent IDs. */
  TrackIDMap trackIDMap_;

  /** Map of track IDs to Trajectory objects. */
  Trajectory::TrajectoryMap trajectoryMap_;
};

}  // namespace simcore

#endif

#ifndef SIMCORE_TRACKMAP_H_
#define SIMCORE_TRACKMAP_H_

// STL
#include <unordered_map>

// Geant4
#include "G4Event.hh"
#include "G4Track.hh"

// LDMX
#include "SimCore/Event/SimParticle.h"
#include "SimCore/UserPrimaryParticleInformation.h"
#include "SimCore/UserTrackInformation.h"

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
   * Add a record in the map for the input track.
   * @param track G4Track to insert
   */
  void insert(const G4Track* track);

  /**
   * Check if the passed track has already been inserted
   * into the track map.
   */
  inline bool contains(const G4Track* track) const {
    return ancestry_.find(track->GetTrackID()) != ancestry_.end();
  }

  /**
   * Find a trajectory's nearest parent that is incident on the calorimeter
   * region. We assume that the primary particles have a parent ID of 0.
   *
   * If this track ID does not have such a trajectory, then the
   * track ID of the primary in its parentage is returned.
   *
   * @param trackID The track ID to search its parentage for the incident
   */
  int findIncident(int trackID) const;

  /**
   * Return true if the given track ID  is saved
   * i.e. will be stored in output file
   *
   * @param trackID The track ID.
   * @return True if the track ID has been inserted in output particle map
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
  std::map<int, ldmx::SimParticle>& getParticleMap() { return particle_map_; }

 private:
  /**
   * Was the input track generated inside the calorimeter region?
   *
   * We rely on the fact that the calorimeter region is named
   *  'CalorimeterRegion'
   * and no other region names contain the string 'Calorimeter'
   */
  bool isInCalorimeterRegion(const G4Track* track) const;

 private:
  /**
   * ancestry map of particles in event (child -> parent)
   *
   * Primary particles are given a "parent" ID of 0 to reflect
   * that they don't have a parent. This is the default in Geant4
   * and we assume that holds here.
   *
   * This is helpful for the findIncident method which looks
   * up through a track's history to find the first ancestor
   * which originated outside of the calorimeter region.
   *
   * The key value is a pair where the first entry
   * is the parent track ID and the second entry is
   * whether **the child track** is in the calorimeter region.
   *
   * @see isInCalorimeterRegion for how we check if a track
   * originated in the calorimeter region.
   */
  std::unordered_map<int, std::pair<int, bool>> ancestry_;

  /// descendents map of particles in event (parent -> children)
  std::unordered_map<int, std::vector<int>> descendents_;

  /// map of SimParticles that will be stored
  std::map<int, ldmx::SimParticle> particle_map_;
};

}  // namespace simcore

#endif

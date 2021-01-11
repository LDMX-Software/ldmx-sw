/**
 * @file G4TrackerHit.h
 * @brief Class defining a tracker hit which is used to create output
 * SimTrackerHit collection
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef SIMCORE_G4SIMTRACKERHIT_H_
#define SIMCORE_G4SIMTRACKERHIT_H_

// Geant4
#include "G4Allocator.hh"
#include "G4THitsCollection.hh"
#include "G4ThreeVector.hh"
#include "G4VHit.hh"

// LDMX
#include "SimCore/Event/SimTrackerHit.h"

// STL
#include <ostream>
#include <vector>

using std::vector;

namespace simcore {

/**
 * @class G4TrackerHit
 * @brief Track hit which is used to create output SimTrackerHit collection
 *
 * @note
 * One of these is created for every step in a TrackerSD.  These are basically
 * copied verbatim into SimTrackerHit objects by the RootPersistencyManager.
 */
class G4TrackerHit : public G4VHit {
 public:
  /**
   * Class constructor.
   */
  G4TrackerHit() {}

  /**
   * Class destructor.
   */
  virtual ~G4TrackerHit() {}

  /**
   * Draw the hit in the Geant4 runtime.
   */
  void Draw();

  /**
   * Print out hit information.
   */
  void Print();

  /**
   * Print hit information to an output stream.
   * @param os The output stream.
   * @return The same output stream.
   */
  std::ostream& print(std::ostream& os);

  /**
   * Create a new hit object.
   * @param s The size of the hit object.
   */
  inline void* operator new(size_t s);

  /**
   * Delete a hit object.
   * @param aHit The hit to delete.
   */
  inline void operator delete(void* aHit);

  /**
   * Set the track ID.
   * @param trackID  The track ID.
   */
  void setTrackID(int trackID) { this->trackID_ = trackID; }

  /**
   * Get the track ID.
   * @return The track ID.
   */
  int getTrackID() { return this->trackID_; }

  /**
   * Get the detector ID.
   * @return The detector ID.
   */
  int getID() { return id_; }

  /**
   * Set the detector ID.
   * @param id The detector ID.
   */
  void setID(int id) { this->id_ = id; }

  /**
   * Get the PDG ID.
   * @return The detector ID.
   */
  int getPdgID() { return pdgid_; }

  /**
   * Set the PDG ID.
   * @param id The detector ID.
   */
  void setPdgID(int pdgid) { this->pdgid_ = pdgid; }

  /**
   * Get the layer ID.
   * @return The layer ID.
   */
  int getLayerID() { return layerID_; }

  /**
   * Set the layer ID.
   * @param layerID The layer ID.
   */
  void setLayerID(int layerID) { this->layerID_ = layerID; }

  /**
   * Get the module ID associated with a hit.  This is used to
   * uniquely identify a sensor within a layer.
   * @return The module ID associated with a hit.
   */
  int getModuleID() const { return moduleID_; };

  /**
   * Set the module ID associated with a hit.  This is used to
   * uniquely identify a sensor within a layer.
   * @return moduleID The module ID associated with a hit.
   */
  void setModuleID(const int moduleID) { this->moduleID_ = moduleID; };

  /**
   * Get the energy deposition.
   * @return The energy deposition.
   */
  float getEdep() { return edep_; }

  /**
   * Get the energy .
   * @return The energy .
   */
  float getEnergy() { return energy_; }

  /**
   * Set the energy deposition.
   * @param edep The energy deposition.
   */
  void setEdep(float edep) { this->edep_ = edep; }

  /**
   * Get the global time.
   * @return The global time.
   */
  float getTime() { return time_; }

  /**
   * Set the global time.
   * @param time The global time.
   */
  void setTime(float time) { this->time_ = time; }

  /**
   * Get the XYZ momentum.
   * @return The XYZ momentum.
   */
  const G4ThreeVector& getMomentum() { return momentum_; }

  /**
   * Set the momentum.
   * @param px The X momentum.
   * @param py The Y momentum.
   * @param pz The Z momentum.
   */
  void setMomentum(float px, float py, float pz) {
    momentum_.setX(px);
    momentum_.setY(py);
    momentum_.setZ(pz);
  }

  /**
   * Get the XYZ hit position [mm].
   * @return The hit position.
   */
  const G4ThreeVector& getPosition() { return position_; }

  /**
   * Set the hit position [mm].
   * @param x The X position.
   * @param y The Y position.
   * 2param z The Z position.
   */
  void setPosition(float x, float y, float z) {
    position_.setX(x);
    position_.setY(y);
    position_.setZ(z);
  }

  /**
   * Set the energy.
   * @param edep The energy.
   */
  void setEnergy(float energy) { this->energy_ = energy; }

  /**
   * Get the path length from the pre and post step points [mm].
   * @return The path length.
   */
  float getPathLength() { return pathLength_; }

  /**
   * Set the path length [mm].
   * @pathLength The path length.
   */
  void setPathLength(float pathLength) { this->pathLength_ = pathLength; }

 private:
  /**
   * The track ID.
   */
  G4int trackID_{0};

  /**
   * The detector ID.
   */
  int id_{0};

  /**
   * The detector ID.
   */
  int pdgid_{0};
  /**
   * The layer ID.
   */
  int layerID_{0};

  /** The module ID. */
  int moduleID_{0};

  /**
   * The energy deposition.
   */
  float edep_{0};

  /**
   * The global time.
   */
  float time_{0};

  /**
   * The XYZ momentum.
   */
  G4ThreeVector momentum_;

  /**
   * The XYZ position.
   */
  G4ThreeVector position_;

  /**
   * The energy .
   */
  float energy_{0};

  /**
   * The path length.
   */
  float pathLength_{0};
};

/**
 * Template instantiation of G4 hits collection class.
 */
typedef G4THitsCollection<G4TrackerHit> G4TrackerHitsCollection;

/**
 * Memory allocator for objects of this class.
 */
extern G4Allocator<G4TrackerHit> G4TrackerHitAllocator;

/**
 * Implementation of custom new operator.
 */
inline void* G4TrackerHit::operator new(size_t) {
  void* aHit;
  aHit = (void*)G4TrackerHitAllocator.MallocSingle();
  return aHit;
}

/**
 * Implementation of custom delete operator.
 */
inline void G4TrackerHit::operator delete(void* aHit) {
  G4TrackerHitAllocator.FreeSingle((G4TrackerHit*)aHit);
}

}  // namespace simcore

#endif

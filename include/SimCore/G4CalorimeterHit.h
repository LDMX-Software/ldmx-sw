/**
 * @file G4CalorimeterHit.h
 * @brief Class defining a calorimeter hit which is used to create output
 * SimCalorimeterHit objects
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef SIMCORE_G4CALORIMETERHIT_H_
#define SIMCORE_G4CALORIMETERHIT_H_

// Geant4
#include "G4Allocator.hh"
#include "G4THitsCollection.hh"
#include "G4ThreeVector.hh"
#include "G4VHit.hh"

// LDMX
#include "SimCore/Event/SimCalorimeterHit.h"

namespace simcore {

/**
 * @class G4CalorimeterHit
 * @brief Calorimeter hit which is used to create output SimCalorimeterHit
 * objects
 *
 * @brief
 * One of these is created for every step in a CalorimeterSD.  These hits are
 * combined later at the end of the event by the RootPersistencyManager from
 * matching their detector IDs.
 */
class G4CalorimeterHit : public G4VHit {
 public:
  /**
   * Class constructor.
   */
  G4CalorimeterHit() {}

  /**
   * Class destructor.
   */
  virtual ~G4CalorimeterHit() {}

  /**
   * Draw the hit in the Geant4 runtime.
   */
  void Draw();

  /**
   * Print information about the hit.
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
   * Get the track ID of the hit.
   * @return The track ID of the hit.
   */
  G4int getTrackID() { return trackID_; }

  /**
   * Set the track ID.
   * @param trackID The track ID.
   */
  void setTrackID(int trackID) { this->trackID_ = trackID; }

  /**
   * Get the detector ID of the hit.
   * @return The detector ID of the hit.
   */
  int getID() { return id_; }

  /**
   * Set the detector ID.
   * @param id The detector ID.
   */
  void setID(int id) { this->id_ = id; }

  /**
   * Get the energy deposition of the hit.
   * @return The energy deposition of the hit.
   */
  float getEdep() { return edep_; }

  /**
   * Set the energy deposition.
   * @param edep The energy deposition.
   */
  void setEdep(float edep) { this->edep_ = edep; }

  /**
   * Get the XYZ hit position [mm].
   * @return The hit position.
   */
  const G4ThreeVector& getPosition() { return position_; }

  /**
   * Set the hit position [mm].
   * @param x The X position.
   * @param y The Y position.
   * @param z The Z position.
   */
  void setPosition(const float x, const float y, const float z) {
    position_.set(x, y, z);
  }

  /**
   * Get the global time of the hit [ns].
   * @return The global hit time.
   */
  float getTime() { return time_; }

  /**
   * Set the global hit time [ns].
   * @param time The global hit time.
   */
  void setTime(float time) { this->time_ = time; }

  /**
   * Get the PDG code of the track that made this hit.
   * @return The PDG code of the track that made the hit.
   */
  int getPdgCode() { return pdgCode_; }

  /**
   * Set the PDG code from the track that made the hit.
   * @param pdgCode The PDG code.
   */
  void setPdgCode(int pdgCode) { pdgCode_ = pdgCode; }

 private:
  /**
   * The track ID.
   */
  int trackID_{-1};

  /**
   * The detector ID.
   */
  int id_{0};

  /**
   * The energy deposition.
   */
  double edep_{0};

  /**
   * The hit position.
   */
  G4ThreeVector position_;

  /**
   * The global time.
   */
  float time_{0};

  /**
   * The PDG code.
   */
  int pdgCode_{0};
};

/**
 * Template instantiation of G4 hits collection class.
 */
typedef G4THitsCollection<G4CalorimeterHit> G4CalorimeterHitsCollection;

/**
 * Memory allocator for objects of this class.
 */
extern G4Allocator<G4CalorimeterHit> G4CalorimeterHitAllocator;

/**
 * Implementation of custom new operator.
 */
inline void* G4CalorimeterHit::operator new(size_t) {
  void* aHit;
  aHit = (void*)G4CalorimeterHitAllocator.MallocSingle();
  return aHit;
}

/**
 * Implementation of custom delete operator.
 */
inline void G4CalorimeterHit::operator delete(void* aHit) {
  G4CalorimeterHitAllocator.FreeSingle((G4CalorimeterHit*)aHit);
}

}  // namespace simcore

#endif

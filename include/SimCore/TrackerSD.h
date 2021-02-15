#ifndef SIMCORE_TRACKERSD_H
#define SIMCORE_TRACKERSD_H

/*~~~~~~~~~~~~*/
/*   Geant4   */
/*~~~~~~~~~~~~*/
#include "G4VSensitiveDetector.hh"

/*~~~~~~~~~~~~~~*/
/*   DetDescr   */
/*~~~~~~~~~~~~~~*/
#include "DetDescr/TrackerID.h"

/*~~~~~~~~~~~~~~~~~~~~*/
/*   SimCore   */
/*~~~~~~~~~~~~~~~~~~~~*/
#include "SimCore/G4TrackerHit.h"

namespace simcore {

/**
 * @class TrackerSD
 * @brief Basic sensitive detector for trackers
 *
 * @note
 * This class creates a G4TrackerHit for each step within the subdetector.
 */
class TrackerSD : public G4VSensitiveDetector {
 public:
  /**
   * Class constructor.
   * @param[in] name The name of the sensitive detector.
   * @param[in] collectionName The name of the hits collection.
   * @param[in] subDetID The subdetector ID.
   */
  TrackerSD(G4String name, G4String collectionName, int subDetID);

  /// Destructor
  ~TrackerSD();

  /**
   * Process a step by creating a hit.
   *
   * @param step The step information
   * @param history The readout history.
   */
  G4bool ProcessHits(G4Step* step, G4TouchableHistory* history);

  /**
   * Initialize the sensitive detector.
   * @param hcEvent The hits collections of the event.
   */
  void Initialize(G4HCofThisEvent* hcEvent);

  /**
   * End of event hook.
   * @param hcEvent The hits collections of the event.
   */
  void EndOfEvent(G4HCofThisEvent* hcEvent);

 private:
  /// The output hits collection of G4TrackerHits.
  G4TrackerHitsCollection* hitsCollection_;

  /// The detector ID
  ldmx::SubdetectorIDType subDetID_;

};  // TrackerID

}  // namespace simcore

#endif  // SIMCORE_TRACKERSD_H

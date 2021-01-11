#ifndef SIMCORE_SCORINGPLANESD_H
#define SIMCORE_SCORINGPLANESD_H

/*~~~~~~~~~~~~*/
/*   Geant4   */
/*~~~~~~~~~~~~*/
#include "G4VSensitiveDetector.hh"

/*~~~~~~~~~~~~~~*/
/*   DetDescr   */
/*~~~~~~~~~~~~~~*/
#include "DetDescr/DetectorID.h"

/*~~~~~~~~~~~~~~~~~~~~*/
/*   SimCore   */
/*~~~~~~~~~~~~~~~~~~~~*/
#include "SimCore/G4TrackerHit.h"

// Forward declaration
class G4Step;

namespace simcore {

/**
 * Class defining a basic sensitive detector for scoring planes.
 */
class ScoringPlaneSD : public G4VSensitiveDetector {
 public:
  /**
   * Constructor
   *
   * @param name The name of the sensitive detector.
   * @param collectionID The name of the hits collection.
   * @param subDetID The subdetector ID.
   */
  ScoringPlaneSD(G4String name, G4String colName, int subDetID);

  /** Destructor */
  ~ScoringPlaneSD();

  /**
   * Process a step and create a hit out of it.
   *
   * @param step The current step.
   * @param history The readout history.
   */
  G4bool ProcessHits(G4Step* step, G4TouchableHistory* history);

  /**
   * Initialize the sensitive detector.
   *
   * @param hcEvent The hits collections associated with this event.
   */
  void Initialize(G4HCofThisEvent* hcEvent);

  /**
   * End of event hook.
   *
   * @param hcEvent The hits collections associated with this event.
   */
  void EndOfEvent(G4HCofThisEvent* hcEvent);

 private:
  /** Output hits collection */
  G4TrackerHitsCollection* hitsCollection_{nullptr};

  /** The detector ID. */
  //            DetectorID* detID_{new DefaultDetectorID()};

};  // ScoringPlaneSD

}  // namespace simcore

#endif  // SIMCORE_SCORINGPLANESD_H_

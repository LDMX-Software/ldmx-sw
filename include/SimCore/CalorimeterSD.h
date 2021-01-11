#ifndef SIMCORE_CALORIMETERSD_H
#define SIMCORE_CALORIMETERSD_H

// Geant4
#include "G4VSensitiveDetector.hh"

// LDMX
#include "DetDescr/DetectorID.h"
#include "Framework/Event.h"
#include "SimCore/G4CalorimeterHit.h"

using ldmx::DetectorID;

namespace simcore {

/**
 * @class CalorimeterSD
 * @brief Basic calorimeter sensitive detector
 */
class CalorimeterSD : public G4VSensitiveDetector {
 public:
  /**
   * Class constructor.
   * @param name The name of the calorimeter.
   * @param theCollectionName The name of the hits collection.
   */
  CalorimeterSD(G4String name, G4String theCollectionName);

  /**
   * Class destructor.
   */
  virtual ~CalorimeterSD();

  /**
   * Set the depth into the geometric hierarchy to the layer volume.
   * @param layerDepth The depth into the geometric hierarchy to the layer
   * volume.
   */
  void setLayerDepth(int layerDepth) { this->layerDepth_ = layerDepth; }

  /**
   * Process a step by creating a hit.
   * @param aStep The step information
   * @param ROhist The readout history.
   */
  virtual G4bool ProcessHits(G4Step* aStep, G4TouchableHistory* ROhist) = 0;

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

 protected:
  /**
   * The hits collections of the sensitive detector.
   */
  G4CalorimeterHitsCollection* hitsCollection_;

  /**
   * The subdetector ID.
   */
  int subdet_;

  /**
   * The depth to the layer volume.
   */
  int layerDepth_{2};
};

}  // namespace simcore

#endif

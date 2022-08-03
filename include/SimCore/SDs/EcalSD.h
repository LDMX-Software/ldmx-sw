/**
 * @file EcalSD.h
 * @brief Class defining an ECal sensitive detector using an EcalHexReadout to
 * create the hits
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef SIMCORE_ECALSD_H_
#define SIMCORE_ECALSD_H_

// LDMX
#include "DetDescr/EcalHexReadout.h"
#include "SimCore/SensitiveDetector.h"
#include "SimCore/Event/SimCalorimeterHit.h"
#include "SimCore/G4User/TrackingAction.h"
#include "SimCore/TrackMap.h"

// ROOT
#include "TMath.h"

// Geant4
#include "G4Polyhedra.hh"

namespace simcore {

/**
 * @class EcalSD
 * @brief ECal sensitive detector that uses an EcalHexReadout to create the hits
 */
class EcalSD : public SensitiveDetector {
 public:
  /// Name of output collection of hits
  static const std::string COLLECTION_NAME;

  /**
   * Class constructor.
   * @param name The name of the sensitive detector.
   * @param theCollectionName The name of the hits collection.
   * @param subDetID The subdetector ID.
   */
  EcalSD(const std::string& name,
         simcore::ConditionsInterface& ci,
         const framework::config::Parameters& p);

  /**
   * Class destructor.
   */
  virtual ~EcalSD();

  /**
   * Should the input volume be consider apart of this sensitive detector?
   *
   * @note Dependent on names defined in GDML!
   */
  virtual bool isSensDet(G4LogicalVolume* vol) const final override {
    auto region = vol->GetRegion();
    if (region and region->GetName().contains("CalorimeterRegion")) {
      return vol->GetName().contains("Si");
    }
    return false;
  }

  /**
   * Process steps to create hits.
   * @param aStep The step information.
   * @param ROhist The readout history.
   */
  G4bool ProcessHits(G4Step* aStep, G4TouchableHistory* ROhist);

  /**
   * Add our hits to the event bus.
   */
  virtual void saveHits(framework::Event& event) final override {
    //event.add(COLLECTION_NAME, hits_);
    hits_.clear();
  }

 private:
  /**
   * Return the hit position of a step.
   * X and Y are computed from the midpoint of the step.
   * Z corresponds to the volume's center.
   * @return The hit position from the step.
   * @todo This function is probably slow due to it inspecting the
   * geometry to get the Z position so this should be sped up somehow.
   */
  G4ThreeVector getHitPosition(G4Step* aStep);

 private:
  /**
   * The hex readout defining the cell grid.
   */
  std::unique_ptr<ldmx::EcalHexReadout> hitMap_;

  /**
   * Map of polygonal layers for getting Z positions.
   */
  std::map<G4VSolid*, G4Polyhedron*> polyMap_;

  /// Collection of hits to add to the event
  std::vector<ldmx::SimCalorimeterHit> hits_;
};

}  // namespace simcore

#endif

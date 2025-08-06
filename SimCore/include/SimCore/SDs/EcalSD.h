/**
 * @file EcalSD.h
 * @brief Class defining an ECal sensitive detector using an EcalHexReadout to
 * create the hits_
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef SIMCORE_ECALSD_H_
#define SIMCORE_ECALSD_H_

// LDMX
#include "DetDescr/EcalGeometry.h"
#include "DetDescr/EcalID.h"
#include "SimCore/Event/SimCalorimeterHit.h"
#include "SimCore/G4User/TrackMap.h"
#include "SimCore/G4User/TrackingAction.h"
#include "SimCore/SDs/SensitiveDetector.h"

// Geant4
// TODO: do we really need both the G4Polyhedron and G4Polyhedra includes?
#include "G4Polyhedra.hh"
#include "G4Polyhedron.hh"
#include "G4Step.hh"
#include "G4StepPoint.hh"
#include "G4VSolid.hh"

// ROOT
#include "TMath.h"

namespace simcore {

/**
 * @class EcalSD
 * @brief ECal sensitive detector that uses an EcalHexReadout to create the
 * hits_
 */
class EcalSD : public SensitiveDetector {
 public:
  /// Name of output collection of hits_
  static const std::string COLLECTION_NAME;

  /**
   * Class constructor.
   * @param name The name of the sensitive detector.
   * @param theCollectionName The name of the hits_ collection.
   * @param subDetID The subdetector ID.
   */
  EcalSD(const std::string& name, simcore::ConditionsInterface& ci,
         const framework::config::Parameters& p);

  /**
   * Class destructor.
   */
  virtual ~EcalSD() = default;

  /**
   * Should the input volume be consider apart of this sensitive detector?
   *
   * @note Dependent on names defined in GDML!
   */
  virtual bool isSensDet(G4LogicalVolume* vol) const override {
    auto region = vol->GetRegion();
    if (region and region->GetName().contains("CalorimeterRegion")) {
      return vol->GetName().contains("Si");
    }
    return false;
  }

  /**
   * Process steps to create hits_.
   * @param aStep The step information.
   * @param ROhist The readout history.
   */
  G4bool ProcessHits(G4Step* aStep, G4TouchableHistory* ROhist) override;

  /**
   * Add our hits_ to the event bus.
   */
  virtual void saveHits(framework::Event& event) override;

  /**
   * Clear the map of hits_ we have accumulated
   */
  virtual void OnFinishedEvent() override { hits_.clear(); }

 private:
  /// map of hits_ to add to the event (will be squashed)
  std::map<ldmx::EcalID, ldmx::SimCalorimeterHit> hits_;
  /// enable hit contribs
  bool enableHitContribs_;
  /// compress hit contribs
  bool compressHitContribs_;
  /// maximum track ID to be considered an "origin"
  int max_origin_track_id_;
};

}  // namespace simcore

#endif

/**
 * @file EcalSD.h
 * @brief Class defining an ECal sensitive detector using an EcalHexReadout to
 * create the hits
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef SIMCORE_ECALSD_H_
#define SIMCORE_ECALSD_H_

// LDMX
#include "DetDescr/EcalID.h"
#include "SimCore/Event/SimCalorimeterHit.h"
#include "SimCore/G4User/TrackingAction.h"
#include "SimCore/SensitiveDetector.h"
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
  G4bool ProcessHits(G4Step* aStep, G4TouchableHistory* ROhist) final override;

  /**
   * Add our hits to the event bus.
   */
  virtual void saveHits(framework::Event& event) final override;

  /**
   * Clear the map of hits we have accumulated
   */
  virtual void EndOfEvent() final override {
    hits_.clear();
  }

 private:
  /// map of hits to add to the event (will be squashed)
  std::map<ldmx::EcalID, ldmx::SimCalorimeterHit> hits_;
  /// enable hit contribs
  bool enableHitContribs_;
  /// compress hit contribs
  bool compressHitContribs_;
};

}  // namespace simcore

#endif

#ifndef SIMCORE_HCALSD_H
#define SIMCORE_HCALSD_H

/*~~~~~~~~~~~~~~*/
/*   DetDescr   */
/*~~~~~~~~~~~~~~*/
#include "SimCore/SensitiveDetector.h"
#include "SimCore/Event/SimCalorimeterHit.h"
#include "SimCore/G4User/TrackingAction.h"
#include "SimCore/TrackMap.h"

namespace simcore {

/**
 * Class defining a sensitive detector of type HCal.
 */
class HcalSD : public SensitiveDetector {
 public:
  /// name of collection to be added to event bus
  static const std::string COLLECTION_NAME;

  /**
   * Constructor
   *
   * @param name The name of the sensitive detector.
   * @param ci Conditions interface handle
   * @param params python configuration parameters
   */
  HcalSD(const std::string& name, simcore::ConditionsInterface& ci,
         const framework::config::Parameters& params);

  /// Destructor
  ~HcalSD();

  /**
   * Check if the input logical volume is a part of the hcal sensitive
   * volumes.
   *
   * @note Depends on the volume names defined in the GDML!
   */
  bool isSensDet(G4LogicalVolume* volume) const final override {
    auto region = volume->GetRegion();
    if (region and region->GetName().contains("CalorimeterRegion")) {
      return volume->GetName().contains("ScintBox");
    }
    return false;
  }

  /**
   * Create a hit out of the energy deposition deposited during a
   * step.
   *
   * @param[in] step The current step.
   * @param[in] history The readout history.
   */
  virtual G4bool ProcessHits(G4Step* aStep,
                             G4TouchableHistory* ROhist) final override;

  /**
   * Add our hits to the event bus and then reset the container
   */
  virtual void saveHits(framework::Event& event) final override {
    event.add(COLLECTION_NAME, hits_);
  }

  virtual void EndOfEvent() final override {
    hits_.clear();
  }

 private:
  // TODO: document!
  double birksc1_;

  // TODO: document!
  double birksc2_;

  // collection of hits to write to event bus
  std::vector<ldmx::SimCalorimeterHit> hits_;

};  // HcalSD

}  // namespace simcore

#endif  // SIMCORE_HCALSD_H

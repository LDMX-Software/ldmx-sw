#ifndef SIMCORE_HCALSD_H
#define SIMCORE_HCALSD_H

/*~~~~~~~~~~~~~~*/
/*   DetDescr   */
/*~~~~~~~~~~~~~~*/
#include <algorithm>
#include <map>
#include <string>
#include <vector>

#include "DetDescr/HcalID.h"
#include "DetDescr/PackedIndex.h"
#include "SimCore/Event/SimCalorimeterHit.h"
#include "SimCore/G4User/TrackMap.h"
#include "SimCore/G4User/TrackingAction.h"
#include "SimCore/SDs/SensitiveDetector.h"

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
  virtual ~HcalSD() = default;

  /**
   * Check if the input logical volume is a part of the hcal sensitive
   * volumes.
   *
   * @note This will match if a) the volume has the auxiliary tag "Region" set
   * to contain to "CalorimeterRegion" and b) the volume name contains one of
   * the identifiers in the gdml_identifiers parameter
   */
  bool isSensDet(G4LogicalVolume* volume) const override {
    auto region = volume->GetRegion();
    if (region and region->GetName().contains("CalorimeterRegion")) {
      const auto name{volume->GetName()};
      return std::find_if(std::begin(gdml_identifiers_),
                          std::end(gdml_identifiers_),
                          [&name](const auto& identifier) {
                            return name.contains(identifier);
                          }) != std::end(gdml_identifiers_);
    }
    return false;
  }

  /**
   * Decode copy number of scintillator bar.
   *
   * @param copyNumber The copy number of the scintillator volume.
   * @param localPosition The position of the hit (step mid-point).
   * @param scint The G4Box of the scintillator volume.
   */
  ldmx::HcalID decodeCopyNumber(const std::uint32_t copyNumber,
                                const G4ThreeVector& localPosition,
                                const G4Box* scint);

  /**
   * Create a hit out of the energy deposition deposited during a
   * step.
   *
   * @param[in] step The current step.
   * @param[in] history The readout history.
   */
  virtual G4bool ProcessHits(G4Step* aStep,
                             G4TouchableHistory* ROhist) override;

  /**
   * Add our hits to the event bus and then reset the container
   */
  virtual void saveHits(framework::Event& event) override;

  virtual void onFinishedEvent() override { hits_.clear(); }

 private:
  // A list of identifiers used to find out whether or not a given logical
  // volume is one of the Hcal sensitive detector volumes. Any volume that is
  // part of the CalorimeterRegion region and has a name which contains at least
  // one of the identifiers in here will be considered a sensitive detector in
  // the Hcal.
  std::vector<std::string> gdml_identifiers_;
  // TODO: document!
  double birksc1_;

  // TODO: document!
  double birksc2_;

  /// map of hits to add to the event (will be squashed)
  std::map<ldmx::HcalID, ldmx::SimCalorimeterHit> hits_;
  /// enable hit contribs
  bool enable_hit_contribs_;
  /// compress hit contribs
  bool compress_hit_contribs_;
  /// maximum track ID to be considered an "origin"
  int max_origin_track_id_;

};  // HcalSD

}  // namespace simcore

#endif  // SIMCORE_HCALSD_H

// VolumeTests.cxx
#include "SimCore/VolumeTests.h"

namespace simcore {
namespace volume_tests {

  /**
  * isInEcal
  *
  * Check that the passed volume is inside the ECal
  *
  * @TODO this is _horrible_
  * can we get an 'ecal' and 'hcal' region instead
  * of just a 'CalorimeterRegion' region?
  *
  * @param[in] vol G4LogicalVolume to check
  * @param[in] vol_to_bias UNUSED name of volume to bias
  */
  bool isInEcal(G4LogicalVolume* vol, const std::string& vol_to_bias) {
    const G4String& volumeName = vol->GetName();
    return ((volumeName.contains("Si") || volumeName.contains("W") ||
             volumeName.contains("PCB") || volumeName.contains("strongback") ||
             volumeName.contains("Glue") || volumeName.contains("CFMix") ||
             volumeName.contains("Al") || volumeName.contains("C")) &&
             volumeName.contains("volume")) ||
             (volumeName.contains("nohole_motherboard"));
  }

  /**
  * isInHcal
  *
  * Check that the passed volume is inside the HCal
  *
  * @param[in] vol G4LogicalVolume to check
  * @param[in] vol_to_bias UNUSED name of volume to bias
  */
  static bool isInHcal(G4LogicalVolume* vol, const std::string& vol_to_bias) {
    const G4String& volumeName = vol->GetName();
    return ((volumeName.contains("abso") || volumeName.contains("ScintBox") ||
             volumeName.contains("scint")) &&
             volumeName.contains("hcal") && volumeName.contains("olume"));
  }

  /**
  * isInEcalOld
  *
  * This is the old method for checking if the passed volume was inside the ECal
  * and only looks for tungsten or silicon layers.
  *
  * @note Deprecating soon (hopefully).
  *
  * @param[in] vol G4LogicalVolume to check
  * @param[in] vol_to_bias UNUSED name of volume to bias
  */
  static bool isInEcalOld(G4LogicalVolume* vol, const std::string& vol_to_bias) {
    const G4String& volumeName = vol->GetName();
    return ((volumeName.contains("Si") || volumeName.contains("W")) &&
          volumeName.contains("volume"));
  }

  /**
  * isInTargetRegion
  *
  * Check if the passed volume is inside the target region.
  *
  * @param[in] vol G4LogicalVolume to check
  * @param[in] vol_to_bias UNUSED name of volume to bias
  */
  static bool isInTargetRegion(G4LogicalVolume* vol,
    const std::string& vol_to_bias) {
    auto region = vol->GetRegion();
    return (region and region->GetName().contains("target"));
  }

  /**
  * isInTargetONLY
  *
  * Check if the passed volume is inside the target volume.
  *
  * @note This leaves out the trig scint modules inside the target region.
  *
  * @param[in] vol G4LogicalVolume to check
  * @param[in] vol_to_bias UNUSED name of volume to bias
  */
  static bool isInTargetOnly(G4LogicalVolume* vol,
    const std::string& vol_to_bias) {
    return vol->GetName().contains("target");
  }

  /**
  * nameContains
  *
  * Check if the passed volume has a name containing the
  * name of the volume to bias.
  *
  * @note This is the default if we don't recognize
  * the volume to bias that is requested.
  *
  * @param[in] vol G4LogicalVolume to check
  * @param[in] vol_to_bias name of volume to bias
  */
  static bool nameContains(G4LogicalVolume* vol, const std::string& vol_to_bias) {
    return vol->GetName().contains(vol_to_bias);
  }

} // namespace volume_tests
} // namespace simcore
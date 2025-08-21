#pragma once

#include "Framework/Exception/Exception.h"
#include "G4LogicalVolume.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4Region.hh"
#include "G4String.hh"

namespace simcore {
namespace g4user {
namespace volumechecks {

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
inline bool isInEcal(G4LogicalVolume* vol, const std::string& vol_to_bias) {
  const G4String& volume_name = vol->GetName();
  return ((volume_name.contains("Si") || volume_name.contains("W") ||
           volume_name.contains("PCB") || volume_name.contains("strongback") ||
           volume_name.contains("Glue") || volume_name.contains("CFMix") ||
           volume_name.contains("Al") || volume_name.contains("C")) &&
          volume_name.contains("volume")) ||
         (volume_name.contains("nohole_motherboard"));
}

/**
 * isInHcal
 *
 * Check that the passed volume is inside the HCal
 *
 * @param[in] vol G4LogicalVolume to check
 * @param[in] vol_to_bias UNUSED name of volume to bias
 */
inline bool isInHcal(G4LogicalVolume* vol, const std::string& vol_to_bias) {
  const G4String& volume_name = vol->GetName();
  return ((volume_name.contains("abso") || volume_name.contains("ScintBox") ||
           volume_name.contains("scint")) &&
          volume_name.contains("hcal") && volume_name.contains("olume"));
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
inline bool isInEcalOld(G4LogicalVolume* vol, const std::string& vol_to_bias) {
  const G4String& volume_name = vol->GetName();
  return ((volume_name.contains("Si") || volume_name.contains("W")) &&
          volume_name.contains("volume"));
}

/**
 * isInTargetRegion
 *
 * Check if the passed volume is inside the target region.
 *
 * @param[in] vol G4LogicalVolume to check
 * @param[in] vol_to_bias UNUSED name of volume to bias
 */
inline bool isInTargetRegion(G4LogicalVolume* vol,
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
inline bool isInTargetOnly(G4LogicalVolume* vol,
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
inline bool nameContains(G4LogicalVolume* vol, const std::string& vol_to_bias) {
  return vol->GetName().contains(vol_to_bias);
}

}  // namespace volumechecks
}  // namespace g4user
}  // namespace simcore

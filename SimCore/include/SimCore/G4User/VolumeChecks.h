#pragma once

#include "G4LogicalVolume.hh"
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
  return ((G4StrUtil::contains(volume_name, "Si") ||
           G4StrUtil::contains(volume_name, "W") ||
           G4StrUtil::contains(volume_name, "PCB") ||
           G4StrUtil::contains(volume_name, "strongback") ||
           G4StrUtil::contains(volume_name, "Glue") ||
           G4StrUtil::contains(volume_name, "CFMix") ||
           G4StrUtil::contains(volume_name, "Al") ||
           G4StrUtil::contains(volume_name, "C")) &&
          G4StrUtil::contains(volume_name, "volume")) ||
         (G4StrUtil::contains(volume_name, "nohole_motherboard"));
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
  return ((G4StrUtil::contains(volume_name, "abso") ||
           G4StrUtil::contains(volume_name, "ScintBox") ||
           G4StrUtil::contains(volume_name, "scint")) &&
          G4StrUtil::contains(volume_name, "hcal") &&
          G4StrUtil::contains(volume_name, "olume"));
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
  return ((G4StrUtil::contains(volume_name, "Si") ||
           G4StrUtil::contains(volume_name, "W")) &&
          G4StrUtil::contains(volume_name, "volume"));
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
  return (region and G4StrUtil::contains(region->GetName(), "target"));
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
  return G4StrUtil::contains(vol->GetName(), "target");
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
  return G4StrUtil::contains(vol->GetName(), std::string_view(vol_to_bias));
}

}  // namespace volumechecks
}  // namespace g4user
}  // namespace simcore

#ifndef SIMCORE_VOLUMETESTS_H
#define SIMCORE_VOLUMETESTS_H

#include <string>

#include "G4LogicalVolume.hh"

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
// bool isInEcal(const G4LogicalVolume* volume);
bool isInEcal(G4LogicalVolume* volume, const std::string& volumeName);

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
bool isInEcalOld(const G4LogicalVolume* volume);

/**
 * isInHcal
 *
 * Check that the passed volume is inside the HCal
 *
 * @param[in] vol G4LogicalVolume to check
 * @param[in] vol_to_bias UNUSED name of volume to bias
 */
bool isInHcal(const G4LogicalVolume* volume);

/**
 * isInTargetRegion
 *
 * Check if the passed volume is inside the target region.
 *
 * @param[in] vol G4LogicalVolume to check
 * @param[in] vol_to_bias UNUSED name of volume to bias
 */
bool isInTargetRegion(const G4LogicalVolume* volume);

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
bool isInTargetOnly(const G4LogicalVolume* volume);

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
bool nameContains(const G4LogicalVolume* volume);

}  // namespace volume_tests
}  // namespace simcore
#endif

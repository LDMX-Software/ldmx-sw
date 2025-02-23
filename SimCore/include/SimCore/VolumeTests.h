// VolumeTests.h
#ifndef SIMCORE_VOLUMETESTS_H
#define SIMCORE_VOLUMETESTS_H

#include "G4LogicalVolume.hh"
#include <string>

namespace simcore {
namespace volume_tests {

/**
 * Check if the given volume is in the ECal region
 * @param volume The G4LogicalVolume to test
 * @return true if the volume is in the ECal
 */
bool isInEcal(const G4LogicalVolume* volume);

/**
 * Check if the given volume is in the HCal region
 * @param volume The G4LogicalVolume to test
 * @return true if the volume is in the HCal
 */
bool isInHcal(const G4LogicalVolume* volume);

/**
 * Check if the given volume is in the target region
 * @param volume The G4LogicalVolume to test
 * @return true if the volume is in the target region
 */
bool isInTargetRegion(const G4LogicalVolume* volume);

} // namespace volume_tests
} // namespace simcore

#endif

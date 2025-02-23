// VolumeTests.cxx
#include "SimCore/VolumeTests.h"

namespace simcore {
namespace volume_tests {

bool isInEcal(const G4LogicalVolume* volume) {
  if (!volume) return false;
  const std::string name = volume->GetName();
  // Copy your existing ECal testing logic here
  return (name.find("ECal") != std::string::npos);
}

bool isInHcal(const G4LogicalVolume* volume) {
  if (!volume) return false;
  const std::string name = volume->GetName();
  // Copy your existing HCal testing logic here
  return (name.find("HCal") != std::string::npos);
}

bool isInTargetRegion(const G4LogicalVolume* volume) {
  if (!volume) return false;
  const std::string name = volume->GetName();
  // Copy your existing target region testing logic here
  return (name.find("Target") != std::string::npos);
}

} // namespace volume_tests
} // namespace simcore
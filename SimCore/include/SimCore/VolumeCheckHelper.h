#ifndef SIMCORE_VOLUMETESTS_H
#define SIMCORE_VOLUMETESTS_H

#include <string>
#include <iostream>


#include "G4Gamma.hh"
#include "G4Region.hh"
#include "G4RegionStore.hh"
#include "G4LogicalVolume.hh"
#include "G4ProcessManager.hh"
#include "G4VPhysicalVolume.hh"
#include "G4TouchableHistory.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4ParticleDefinition.hh"
#include "G4PhysicalVolumeStore.hh"


namespace simcore {
namespace VolumeCheckHelper {

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




/**
 * @brief Retrieve a specific process for a given particle type.
 * @param particle Pointer to the G4ParticleDefinition object.
 * @param processName Name of the process to find.
 * @return Pointer to the process if found, nullptr otherwise.
 */
inline const G4VProcess* getProcess(const G4ParticleDefinition* particle,
                                    const std::string& processName) {
  if (!particle) return nullptr;

  const auto manager = particle->GetProcessManager();
  if (!manager) return nullptr;

  const auto processes = manager->GetProcessList();
  for (int i = 0; i < processes->size(); ++i) {
    const auto process = (*processes)[i];
    if (process->GetProcessName().find(processName) != std::string::npos) {
      return process;
    }
  }
  return nullptr;  // No matching process found
}

/**
 * @brief Retrieve the photonuclear process for gamma particles.
 * @return Pointer to the photonuclear process if found, nullptr otherwise.
 */
inline const G4VProcess* getPhotonuclearProcess() {
  return GetProcess(G4Gamma::Definition(), "photonNuclear");
}

/**
 * @brief Retrieve a Geant4 region by name.
 * @param name Name of the region.
 * @return Pointer to the region if found, nullptr otherwise.
 */
inline G4Region* getRegion(const std::string& name) {
  return G4RegionStore::GetInstance()->GetRegion(name);
}

/**
 * @brief Retrieve a Geant4 physical volume by name.
 * @param name Name of the physical volume.
 * @return Pointer to the physical volume if found, nullptr otherwise.
 */
inline G4VPhysicalVolume* getVolume(const std::string& name) {
  auto* volumeStore = G4PhysicalVolumeStore::GetInstance();
  for (const auto& volume : *volumeStore) {
    if (std::string(volume->GetName()) ==
        name) {  // Convert G4String to std::string
      return volume;
        }
  }
  return nullptr;  // Volume not found
}

}  // namespace VolumeCheckHelper
}  // namespace simcore
#endif

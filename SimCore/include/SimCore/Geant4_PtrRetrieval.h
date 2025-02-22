#ifndef SIMCORE_GEANT4_PTR_RETRIEVAL_H
#define SIMCORE_GEANT4_PTR_RETRIEVAL_H

#include "G4ParticleDefinition.hh"
#include "G4ProcessManager.hh"
#include "G4LogicalVolume.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4RegionStore.hh"
#include "G4Region.hh"
#include "G4Gamma.hh"
#include "G4VPhysicalVolume.hh"
#include <string>
#include <iostream>

#include "G4RegionStore.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4TouchableHistory.hh"

namespace Geant4_PtrRetrieval {

/**
 * @brief Retrieve a specific process for a given particle type.
 * @param particle Pointer to the G4ParticleDefinition object.
 * @param processName Name of the process to find.
 * @return Pointer to the process if found, nullptr otherwise.
 */
inline const G4VProcess* GetProcess(const G4ParticleDefinition* particle, const std::string& processName) {
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
inline const G4VProcess* GetPhotonuclearProcess() {
  return GetProcess(G4Gamma::Definition(), "photonNuclear");
}

/**
 * @brief Retrieve a Geant4 region by name.
 * @param name Name of the region.
 * @return Pointer to the region if found, nullptr otherwise.
 */
inline G4Region* GetRegion(const std::string& name) {
    return G4RegionStore::GetInstance()->GetRegion(name);
}

/**
 * @brief Retrieve a Geant4 physical volume by name.
 * @param name Name of the physical volume.
 * @return Pointer to the physical volume if found, nullptr otherwise.
 */
inline G4VPhysicalVolume* GetVolume(const std::string& name) {
  auto* volumeStore = G4PhysicalVolumeStore::GetInstance();
  for (const auto& volume : *volumeStore) {
    if (std::string(volume->GetName()) == name) {  // Convert G4String to std::string
      return volume;
    }
  }
  return nullptr;  // Volume not found
}

} // namespace Geant4_ptr_retrieval

#endif // SIMCORE_PTR_RETRIEVAL_H

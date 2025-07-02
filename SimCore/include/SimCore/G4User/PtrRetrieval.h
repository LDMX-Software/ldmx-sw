#ifndef SIMCORE_PTRRETRIEVAL_H
#define SIMCORE_PTRRETRIEVAL_H

//----------------//
//   C++ StdLib   //
//----------------//
#include <iostream>
#include <string>

//------------//
//   Geant4   //
//------------//
#include "G4Gamma.hh"
#include "G4LogicalVolume.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4ParticleDefinition.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4ProcessManager.hh"
#include "G4Region.hh"
#include "G4RegionStore.hh"
#include "G4VPhysicalVolume.hh"

namespace simcore {
namespace g4user {
namespace ptrretrieval {

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
  return nullptr;
}

/**
 * @brief Retrieve the photonuclear process for gamma particles.
 * @return Pointer to the photonuclear process if found, nullptr otherwise.
 */
inline const G4VProcess* getPhotonuclearProcess() {
  return getProcess(G4Gamma::Definition(), "photonNuclear");
}

/**
 * @brief Retrieve a Geant4 region by name.
 * @param name Name of the region.
 * @return Pointer to the region if found, nullptr otherwise.
 *
 * It can be helpful to store a copy of the pointer in a local
 * variable so that the search doesn't need to be performed every time
 * a comparison needs to be made.
 * In many cases, this can be achieved by a `static` variable that
 * will call this function when it first needs to be constructed
 * and then just uses that value for the rest of running.
 * ```cpp
 * static auto reg = simcore::g4user::ptrretrieval::getRegion("MyRegion");
 * ```
 */
inline G4Region* getRegion(const std::string& name) {
  G4Region* region = G4RegionStore::GetInstance()->GetRegion(name);
  if (!region) {
    return nullptr;
  }
  return region;
}

/**
 * @brief Retrieve a Geant4 physical volume by name.
 * @param name Name of the physical volume.
 * @return Pointer to the physical volume if found, nullptr otherwise.
 */
inline G4VPhysicalVolume* getPhysicalVolume(const std::string& name) {
  auto* volume = G4PhysicalVolumeStore::GetInstance()->GetVolume(name);
  if (!volume) {
    return nullptr;
  }
  return volume;
}

/**
 * @brief Retrieve a Geant4 logical volume by name.
 * @param name Name of the logical volume.
 * @return Pointer to the logical volume if found, nullptr otherwise.
 */
inline G4LogicalVolume* getLogicalVolume(const std::string& name) {
  auto* volume = G4LogicalVolumeStore::GetInstance()->GetVolume(name);
  if (!volume) {
    return nullptr;
  }
  return volume;
}

/**
 * @brief Retrieve a Geant4 material by name.
 * @param name Name of the material.
 * @return Pointer to the material if found, nullptr otherwise.
 *
 * This function retrieves a material from the G4Material store
 * using its name. If the material is not found, it returns nullptr.
 */
inline G4Material* getMaterial(const std::string& name) {
  auto* material = G4Material::GetMaterial(name);
  if (!material) {
    return nullptr;
  }
  return material;
}

}  // namespace ptrretrieval
}  // namespace g4user
}  // namespace simcore
#endif  // SIMCORE_PTRRETRIEVAL_H

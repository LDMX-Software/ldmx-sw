/**
 * @file GammaPhysics.cxx
 * @brief Class used to enhanced the gamma physics list.
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "SimCore/GammaPhysics.h"

namespace simcore {

// needed for GEANT4 10.3.0 and later
//
// TODO: I (Einar) don't think this really is necessary if we change
// GetGammaProcessManager below
#ifndef aParticleIterator
#define aParticleIterator \
  ((subInstanceManager.offset[g4vpcInstanceID])._aParticleIterator)
#endif

G4ProcessManager* GammaPhysics::GetGammaProcessManager() const {
  /**
   * Not entirely clear that there isn't a simpler way to do this, but to keep
   * this function extraction to a pure refactoring, I'm leaving it here for
   * now. /Einar
   */
  aParticleIterator->reset();

  // Loop through all of the particles and find the "gamma".
  while ((*aParticleIterator)()) {
    auto particle{aParticleIterator->value()};
    auto particleName{particle->GetParticleName()};
    if (particleName == "gamma") {
      return particle->GetProcessManager();
    }
  }
  EXCEPTION_RAISE("GammaPhysics",
                  "Was unable to access the process manager for photons, "
                  "something is very wrong!");
}

void GammaPhysics::SetPhotonNuclearAsFirstProcess() const {
  auto processManager{GetGammaProcessManager()};
  // Get the process list associated with the gamma.
  const auto processes{processManager->GetProcessList()};

  for (int i{0}; i < processes->size(); ++i) {
    const auto process{(*processes)[i]};
    const auto processName{process->GetProcessName()};
    if (processName == "photonNuclear") {
      processManager->SetProcessOrderingToFirst(
          process, G4ProcessVectorDoItIndex::idxAll);
    }
  }
}
void GammaPhysics::ConstructProcess() {
  G4ProcessManager* processManager = GetGammaProcessManager();

  auto pn = PhotoNuclearModel::Factory::get().make(
      modelParameters.getParameter<std::string>("class_name"),
      modelParameters.getParameter<std::string>("instance_name"),
      modelParameters);
  pn->removeExistingModel(processManager);
  pn->ConstructGammaProcess(processManager);
  SetPhotonNuclearAsFirstProcess();
  // Add the gamma -> mumu to the physics list.
  processManager->AddDiscreteProcess(&gammaConvProcess);
}
}  // namespace simcore

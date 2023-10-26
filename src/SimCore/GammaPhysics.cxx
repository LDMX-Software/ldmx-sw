/**
 * @file GammaPhysics.cxx
 * @brief Class used to enhanced the gamma physics list.
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "SimCore/GammaPhysics.h"

namespace simcore {

GammaPhysics::GammaPhysics(const G4String& name,
                           const framework::config::Parameters& parameters)
    : G4VPhysicsConstructor(name),
      modelParameters{parameters.getParameter<framework::config::Parameters>(
          "photonuclear_model")} {}

void GammaPhysics::ConstructParticle() {}

void GammaPhysics::ConstructProcess() {
  G4ProcessManager* processManager = G4Gamma::Gamma()->GetProcessManager();
  if (processManager == nullptr) {
    EXCEPTION_RAISE("GammaPhysics",
                    "Was unable to access the process manager for photons, "
                    "something is very wrong!");
  }
  // configure our PN model based on runtime parameters
  auto pn = PhotoNuclearModel::Factory::get().make(
      modelParameters.getParameter<std::string>("class_name"),
      modelParameters.getParameter<std::string>("instance_name"),
      modelParameters);
  pn->removeExistingModel(processManager);
  pn->ConstructGammaProcess(processManager);
  /**
   * Put the PN process first in the ordering in case PN biasing is happening.
   *
   * Process ordering is a complicated concept and unfortunately
   * its affect on biasing is poorly documented. What has been said is
   * that some processes _need_ to be called _last_[1]. In addition,
   * the practical experience of working on defining a custom
   * process for
   * [G4DarkBreM](https://github.com/LDMX-Software/G4DarkBreM)
   * showed that sometimes Geant4 does
   * not get through the full list of processes but it always starts
   * at the beginning. For these reasons, we put the PN process first
   * in the ordering so that we can insure it is always check by Geant4
   * before continuing.
   *
   * [1]
   * https://indico.cern.ch/event/58317/contributions/2047449/attachments/992300/1411062/PartDef-ProcMan-AS.pdf
   */
  const auto processes{processManager->GetProcessList()};
  for (int i{0}; i < processes->size(); i++) {
    const auto process{(*processes)[i]};
    if (process->GetProcessName() == "photonNuclear") {
      processManager->SetProcessOrderingToFirst(
          process, G4ProcessVectorDoItIndex::idxAll);
    }
  }
  // Add the gamma -> mumu to the physics list.
  processManager->AddDiscreteProcess(&gammaConvProcess);
}
}  // namespace simcore

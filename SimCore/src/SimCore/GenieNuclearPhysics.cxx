/**
 * @file GenieNuclearPhysics.cxx
 * @brief Implementation of the GenieNuclearPhysics constructor.
 */

#include "SimCore/GenieNuclearPhysics.h"

#include "SimCore/GenieElectroNuclearProcess.h"

#include "G4Electron.hh"
#include "G4ProcessManager.hh"
#include "G4ProcessTable.hh"

namespace simcore {

const std::string GenieNuclearPhysics::NAME = "GenieNuclear";

GenieNuclearPhysics::GenieNuclearPhysics(
    const framework::config::Parameters& params)
    : G4VPhysicsConstructor(NAME), params_{params} {
  enable_ = params_.get<bool>("enable", false);
}

void GenieNuclearPhysics::ConstructParticle() {
  // No new particles — we only replace an existing process
}

void GenieNuclearPhysics::ConstructProcess() {
  if (!enable_) {
    ldmx_log(info) << "GENIE electronuclear process is not enabled";
    return;
  }

  ldmx_log(info) << "Setting up GENIE electronuclear process...";

  G4ProcessManager* process_manager =
      G4Electron::Definition()->GetProcessManager();
  if (!process_manager) {
    EXCEPTION_RAISE("GenieNuclearPhysics",
                    "Unable to access the electron process manager!");
  }

  // Find and remove the built-in electronNuclear process
  G4VProcess* existing =
      G4ProcessTable::GetProcessTable()->FindProcess("electronNuclear", "e-");
  if (existing) {
    ldmx_log(info) << "Removing built-in electronNuclear process";
    process_manager->RemoveProcess(existing);
  } else {
    ldmx_log(info)
        << "No built-in electronNuclear process found — adding GENIE process";
  }

  // Create and register the GENIE-based process
  auto* genie_process = new GenieElectroNuclearProcess(params_);
  process_manager->AddDiscreteProcess(genie_process);

  // Set ordering to first so biasing framework always sees it
  process_manager->SetProcessOrderingToFirst(
      genie_process, G4ProcessVectorDoItIndex::idxAll);

  ldmx_log(info) << "GENIE electronuclear process registered successfully";
}

}  // namespace simcore

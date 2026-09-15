#include "SimCore/FCPPhysics.h"

namespace simcore {

const std::string FCPPhysics::NAME = "FCPPhysics";

FCPPhysics::FCPPhysics(const G4String& name,
                       const framework::config::Parameters& parameters)
    : G4VPhysicsConstructor(name) {
  enable_ = parameters.get<bool>("enable", false);
  fcp_mass_ = parameters.get<double>("fcp_mass", 0.) * MeV;
  fcp_charge_ = parameters.get<double>("fcp_charge", 0.1);
  fcp_pdg_id_ = parameters.get<int>("fcp_pdg_id", 17);

  ldmx_log(info) << "FCPPhysics configuration:" << " Enabled: "
                 << (enable_ ? "YES" : "NO")
                 << ", FCP mass: " << fcp_mass_ / MeV << " MeV"
                 << ", FCP charge: " << fcp_charge_ << " e"
                 << ", FCP PDG ID: " << fcp_pdg_id_;
}

void FCPPhysics::ConstructParticle() {
  if (!enable_) return;

  ldmx_log(info) << "Initializing FCP particles with mass " << fcp_mass_ / MeV
                 << " MeV, charge " << fcp_charge_ << " e, PDG ID "
                 << fcp_pdg_id_;
  G4FractionallyCharged::Initialize(fcp_mass_, fcp_pdg_id_, fcp_charge_);
}

void FCPPhysics::ConstructProcess() {
  if (!enable_) return;

  ldmx_log(debug) << "Setting up gamma -> fcp+ fcp- conversion process...";

  G4ProcessManager* gamma_proc_man = G4Gamma::Gamma()->GetProcessManager();
  if (gamma_proc_man == nullptr) {
    ldmx_log(error) << "FATAL: Unable to access process manager for gamma!";
    EXCEPTION_RAISE("FCPPhysics",
                    "Was unable to access the process manager for gamma, "
                    "something is very wrong!");
  }

  gamma_fcp_process_ = new GammaConversionToFCPs();
  gamma_proc_man->AddDiscreteProcess(gamma_fcp_process_);
  ldmx_log(info)
      << "gamma -> fcp+ fcp- conversion process registered successfully!";
}

}  // namespace simcore

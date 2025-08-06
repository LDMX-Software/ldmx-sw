#include "SimCore/BiasOperators/XsecBiasingOperator.h"

#include "Framework/Exception/Exception.h"

namespace simcore {

XsecBiasingOperator::XsecBiasingOperator(
    std::string name, const framework::config::Parameters& parameters)
    : G4VBiasingOperator(name), the_log_{framework::logging::makeLogger(name)} {}

void XsecBiasingOperator::StartRun() {
  if (this->getParticleToBias().compare("gamma") == 0) {
    processManager_ = G4Gamma::GammaDefinition()->GetProcessManager();
  } else if (this->getParticleToBias().compare("e-") == 0) {
    processManager_ = G4Electron::ElectronDefinition()->GetProcessManager();
  } else if (this->getParticleToBias().compare("neutron") == 0) {
    processManager_ = G4Neutron::NeutronDefinition()->GetProcessManager();
  } else if (this->getParticleToBias().compare("kaon0L") == 0) {
    processManager_ =
        G4KaonZeroLong::KaonZeroLongDefinition()->GetProcessManager();
  } else {
    EXCEPTION_RAISE("BiasSetup", "Invalid particle type '" +
                                     this->getParticleToBias() + "'.");
  }

  ldmx_log(info) << "Biasing particles of type " << this->getParticleToBias();

  if (processIsBiased(this->getProcessToBias())) {
    xsecOperation_ =
        new G4BOptnChangeCrossSection("changeXsec-" + this->getProcessToBias());
  } else {
    EXCEPTION_RAISE("BiasSetup",
                    this->getProcessToBias() +
                        " is not found in list of biased processes!");
  }
}

bool XsecBiasingOperator::processIsBiased(std::string process) {
  // Loop over all processes and check if the given process is being
  // biased.
  const G4BiasingProcessSharedData* shared_data =
      G4BiasingProcessInterface::GetSharedData(processManager_);
  if (shared_data) {
    for (size_t iprocess = 0;
         iprocess < (shared_data->GetPhysicsBiasingProcessInterfaces()).size();
         ++iprocess) {
      const G4BiasingProcessInterface* wrapper_process =
          (shared_data->GetPhysicsBiasingProcessInterfaces())[iprocess];

      if (wrapper_process->GetWrappedProcess()->GetProcessName().compareTo(
              process) == 0) {
        return true;
      }
    }
  }
  return false;
}

DEFINE_FACTORY(XsecBiasingOperator);

}  // namespace simcore

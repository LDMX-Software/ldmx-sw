
#include "SimCore/BiasOperators/DarkBrem.h"

//------------//
//   Geant4   //
//------------//
#include "G4RunManager.hh"

namespace simcore {
namespace biasoperators {

DarkBrem::DarkBrem(std::string name, const framework::config::Parameters& p)
    : XsecBiasingOperator(name, p) {
  volume_ = p.getParameter<std::string>("volume");
  factor_ = p.getParameter<double>("factor");
  bias_all_ = p.getParameter<bool>("bias_all");
}

G4VBiasingOperation* DarkBrem::ProposeOccurenceBiasingOperation(
    const G4Track* track, const G4BiasingProcessInterface* callingProcess) {
  std::string currentProcess =
      callingProcess->GetWrappedProcess()->GetProcessName();
  if (currentProcess.compare(this->getProcessToBias()) == 0) {
    // bias only the primary particle if we don't want to bias all particles
    if (not bias_all_ and track->GetParentID() != 0) return 0;

    G4double interactionLength =
        callingProcess->GetWrappedProcess()->GetCurrentInteractionLength();

    double dbXsecUnbiased = 1. / interactionLength;
    double dbXsecBiased = dbXsecUnbiased * factor_;

    if (G4RunManager::GetRunManager()->GetVerboseLevel() > 1) {
      std::cout << "[ DarkBremXsecBiasingOperator ]: "
                << " Unbiased DBrem xsec: " << dbXsecUnbiased
                << " -> Biased xsec: " << dbXsecBiased << std::endl;
    }

    return BiasedXsec(dbXsecBiased);
  } else
    return 0;
}

void DarkBrem::RecordConfig(ldmx::RunHeader& h) const {
  h.setIntParameter("BiasOperator::DarkBrem::Bias All Electrons", bias_all_);
  h.setFloatParameter("BiasOperator::DarkBrem::Factor", factor_);
  h.setStringParameter("BiasOperator::DarkBrem::Volume", volume_);
}

}  // namespace biasoperators
}  // namespace simcore

DECLARE_XSECBIASINGOPERATOR(simcore::biasoperators, DarkBrem)

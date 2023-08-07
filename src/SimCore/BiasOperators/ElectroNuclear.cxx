
#include "SimCore/BiasOperators/ElectroNuclear.h"

namespace simcore {
namespace biasoperators {

ElectroNuclear::ElectroNuclear(std::string name,
                               const framework::config::Parameters& p)
    : XsecBiasingOperator(name, p) {
  volume_ = p.getParameter<std::string>("volume");
  factor_ = p.getParameter<double>("factor");
  threshold_ = p.getParameter<double>("threshold");
}

G4VBiasingOperation* ElectroNuclear::ProposeOccurenceBiasingOperation(
    const G4Track* track, const G4BiasingProcessInterface* callingProcess) {
  if (track->GetKineticEnergy() < threshold_) {
    return nullptr;
  }

  std::string currentProcess =
      callingProcess->GetWrappedProcess()->GetProcessName();
  if (currentProcess.compare(this->getProcessToBias()) == 0) {
    G4double interactionLength =
        callingProcess->GetWrappedProcess()->GetCurrentInteractionLength();
    double enXsecUnbiased = 1. / interactionLength;

    double enXsecBiased = enXsecUnbiased * factor_;

    return BiasedXsec(enXsecBiased);
  }
  return nullptr;
}
}  // namespace biasoperators
}  // namespace simcore

DECLARE_XSECBIASINGOPERATOR(simcore::biasoperators::ElectroNuclear)


#include "SimCore/BiasOperators/K0LongInelastic.h"

namespace simcore {
namespace biasoperators {

K0LongInelastic::K0LongInelastic(std::string name, const framework::config::Parameters& p)
    : XsecBiasingOperator(name, p) {
  volume_ = p.getParameter<std::string>("volume");
  factor_ = p.getParameter<double>("factor");
  threshold_ = p.getParameter<double>("threshold");
}

G4VBiasingOperation* K0LongInelastic::ProposeOccurenceBiasingOperation(
    const G4Track* track, const G4BiasingProcessInterface* callingProcess) {
  if (track->GetKineticEnergy() < threshold_) {
    return nullptr;
  };

  std::string currentProcess =
      callingProcess->GetWrappedProcess()->GetProcessName();
  if (currentProcess.compare(this->getProcessToBias()) == 0) {
    G4double interactionLength =
        callingProcess->GetWrappedProcess()->GetCurrentInteractionLength();

    double k0LongInXsecUnbiased = 1. / interactionLength;

    double k0LongInXsecBiased = k0LongInXsecUnbiased * factor_;

    return BiasedXsec(k0LongInXsecBiased);
  }
  return nullptr;
}

}  // namespace biasoperators
}  // namespace simcore

DECLARE_XSECBIASINGOPERATOR(simcore::biasoperators::K0LongInelastic)

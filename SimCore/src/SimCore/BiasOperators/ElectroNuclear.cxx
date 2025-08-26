
#include "SimCore/BiasOperators/ElectroNuclear.h"

namespace simcore {
namespace biasoperators {

ElectroNuclear::ElectroNuclear(std::string name,
                               const framework::config::Parameters& p)
    : XsecBiasingOperator(name, p) {
  volume_ = p.get<std::string>("volume");
  factor_ = p.get<double>("factor");
  threshold_ = p.get<double>("threshold");
}

G4VBiasingOperation* ElectroNuclear::ProposeOccurenceBiasingOperation(
    const G4Track* track, const G4BiasingProcessInterface* callingProcess) {
  if (track->GetKineticEnergy() < threshold_) {
    return nullptr;
  }

  std::string current_process =
      callingProcess->GetWrappedProcess()->GetProcessName();
  if (current_process.compare(this->getProcessToBias()) == 0) {
    G4double interaction_length =
        callingProcess->GetWrappedProcess()->GetCurrentInteractionLength();
    double en_xsec_unbiased = 1. / interaction_length;

    double en_xsec_biased = en_xsec_unbiased * factor_;

    return BiasedXsec(en_xsec_biased);
  }
  return nullptr;
}
}  // namespace biasoperators
}  // namespace simcore

DECLARE_XSECBIASINGOPERATOR(simcore::biasoperators::ElectroNuclear)

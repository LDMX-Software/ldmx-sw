/**
 * @file APrimeToFCPPair.cxx
 * @brief Biasing operator for the A' -> fcp+ fcp- process.
 * @author Tamas Almos Vami, UCSB
 */

#include "SimCore/BiasOperators/APrimeToFCPPair.h"

namespace simcore {
namespace biasoperators {

APrimeToFCPPair::APrimeToFCPPair(std::string name,
                                 const framework::config::Parameters& p)
    : XsecBiasingOperator(name, p) {
  volume_ = p.get<std::string>("volume");
  factor_ = p.get<double>("factor");
  threshold_ = p.get<double>("threshold");
}

G4VBiasingOperation* APrimeToFCPPair::ProposeOccurenceBiasingOperation(
    const G4Track* track, const G4BiasingProcessInterface* callingProcess) {
  if (track->GetKineticEnergy() < threshold_) {
    return nullptr;
  }

  std::string current_process =
      callingProcess->GetWrappedProcess()->GetProcessName();
  if (current_process.compare(this->getProcessToBias()) == 0) {
    G4double interaction_length =
        callingProcess->GetWrappedProcess()->GetCurrentInteractionLength();

    double xsec_unbiased = 1. / interaction_length;
    double xsec_biased = xsec_unbiased * factor_;

    return BiasedXsec(xsec_biased);
  }
  return nullptr;
}

}  // namespace biasoperators
}  // namespace simcore

DECLARE_XSECBIASINGOPERATOR(simcore::biasoperators::APrimeToFCPPair)

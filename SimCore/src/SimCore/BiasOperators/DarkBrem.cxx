
#include "SimCore/BiasOperators/DarkBrem.h"

//------------//
//   Geant4   //
//------------//
#include "G4RunManager.hh"

namespace simcore {
namespace biasoperators {

DarkBrem::DarkBrem(std::string name, const framework::config::Parameters& p)
    : XsecBiasingOperator(name, p) {
  volume_ = p.get<std::string>("volume");
  factor_ = p.get<double>("factor");
  bias_all_ = p.get<bool>("bias_all");
}

G4VBiasingOperation* DarkBrem::ProposeOccurenceBiasingOperation(
    const G4Track* track, const G4BiasingProcessInterface* callingProcess) {
  std::string current_process =
      callingProcess->GetWrappedProcess()->GetProcessName();
  if (current_process.compare(this->getProcessToBias()) == 0) {
    // bias only the primary particle if we don't want to bias all particles
    if (not bias_all_ and track->GetParentID() != 0) return nullptr;

    G4double interaction_length =
        callingProcess->GetWrappedProcess()->GetCurrentInteractionLength();

    double db_xsec_unbiased = 1. / interaction_length;
    double db_xsec_biased = db_xsec_unbiased * factor_;

    if (G4RunManager::GetRunManager()->GetVerboseLevel() > 1) {
      std::cout << "[ DarkBremXsecBiasingOperator ]: "
                << " Unbiased DBrem xsec: " << db_xsec_unbiased
                << " -> Biased xsec: " << db_xsec_biased << std::endl;
    }

    return BiasedXsec(db_xsec_biased);
  }
  return nullptr;
}

void DarkBrem::RecordConfig(ldmx::RunHeader& h) const {
  h.setIntParameter("BiasOperator::DarkBrem::Bias All Electrons", bias_all_);
  h.setFloatParameter("BiasOperator::DarkBrem::Factor", factor_);
  h.setStringParameter("BiasOperator::DarkBrem::Volume", volume_);
}
}  // namespace biasoperators
}  // namespace simcore

DECLARE_XSECBIASINGOPERATOR(simcore::biasoperators::DarkBrem)

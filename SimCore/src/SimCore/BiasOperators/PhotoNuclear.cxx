#include "SimCore/BiasOperators/PhotoNuclear.h"

namespace simcore {
namespace biasoperators {

const std::string PhotoNuclear::CONVERSION_PROCESS = "conv";

PhotoNuclear::PhotoNuclear(std::string name,
                           const framework::config::Parameters& p)
    : XsecBiasingOperator(name, p) {
  volume_ = p.getParameter<std::string>("volume");
  threshold_ = p.getParameter<double>("threshold");
  factor_ = p.getParameter<double>("factor");
  down_bias_conv_ = p.getParameter<bool>("down_bias_conv");
  only_children_of_primary_ = p.getParameter<bool>("only_children_of_primary");
}

void PhotoNuclear::StartRun() {
  XsecBiasingOperator::StartRun();

  if (processIsBiased(CONVERSION_PROCESS)) {
    emXsecOperation = new G4BOptnChangeCrossSection("changeXsec-conv");
  } else if (down_bias_conv_) {
    EXCEPTION_RAISE(
        "PhotoNuclearBiasing",
        "Gamma Conversion process '" + CONVERSION_PROCESS + "' is not biased!");
  }
}

G4VBiasingOperation* PhotoNuclear::ProposeOccurenceBiasingOperation(
    const G4Track* track, const G4BiasingProcessInterface* callingProcess) {
  // if we want to only bias children of primary, leave if this track is NOT a
  // child of the primary
  if (only_children_of_primary_ and track->GetParentID() != 1) {
    return nullptr;
  }

  // is this track too low energy to be biased?
  if (track->GetKineticEnergy() < threshold_) {
    return nullptr;
  }

  std::string currentProcess =
      callingProcess->GetWrappedProcess()->GetProcessName();
  if (currentProcess.compare(this->getProcessToBias()) == 0) {
    G4double interactionLength =
        callingProcess->GetWrappedProcess()->GetCurrentInteractionLength();

    pnXsecUnbiased_ = 1. / interactionLength;

    pnXsecBiased_ = pnXsecUnbiased_ * factor_;

    return BiasedXsec(pnXsecBiased_);
  }
  if ((currentProcess.compare(CONVERSION_PROCESS) == 0) and down_bias_conv_) {
    G4double interactionLength =
        callingProcess->GetWrappedProcess()->GetCurrentInteractionLength();

    double emXsecUnbiased = 1. / interactionLength;

    double emXsecBiased = std::max(
        emXsecUnbiased + pnXsecUnbiased_ - pnXsecBiased_, pnXsecUnbiased_);
    if (emXsecBiased == pnXsecUnbiased_) {
      G4cout << "[ PhotoNuclearXsecBiasingOperator ]: [ WARNING ]: "
             << "Biasing factor is too large." << std::endl;
    }

    emXsecOperation->SetBiasedCrossSection(emXsecBiased);
    emXsecOperation->Sample();

    return emXsecOperation;
  }
  return nullptr;
}

}  // namespace biasoperators
}  // namespace simcore

DECLARE_XSECBIASINGOPERATOR(simcore::biasoperators::PhotoNuclear)

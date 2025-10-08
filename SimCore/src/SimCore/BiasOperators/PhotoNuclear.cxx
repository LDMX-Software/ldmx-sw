#include "SimCore/BiasOperators/PhotoNuclear.h"

namespace simcore {
namespace biasoperators {

const std::string PhotoNuclear::CONVERSION_PROCESS = "conv";

PhotoNuclear::PhotoNuclear(std::string name,
                           const framework::config::Parameters& p)
    : XsecBiasingOperator(name, p) {
  volume_ = p.get<std::string>("volume");
  threshold_ = p.get<double>("threshold");
  factor_ = p.get<double>("factor");
  down_bias_conv_ = p.get<bool>("down_bias_conv");
  only_children_of_primary_ = p.get<bool>("only_children_of_primary");
}

void PhotoNuclear::StartRun() {
  XsecBiasingOperator::StartRun();

  if (processIsBiased(CONVERSION_PROCESS)) {
    em_xsec_operation_ = new G4BOptnChangeCrossSection("changeXsec-conv");
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

  std::string current_process =
      callingProcess->GetWrappedProcess()->GetProcessName();

  // If the current process is what want to bias
  if (current_process.compare(this->getProcessToBias()) == 0) {
    G4double interaction_length =
        callingProcess->GetWrappedProcess()->GetCurrentInteractionLength();

    pn_xsec_unbiased_ = 1. / interaction_length;

    pn_xsec_biased_ = pn_xsec_unbiased_ * factor_;

    return BiasedXsec(pn_xsec_biased_);
  }

  // If the current process is conversion and we want to bias down
  // make sure that the biased electromagnetic xsec never falls below the
  // normal (unbiased) photon-nuclear cross section
  // In that regime, the PN events generated would almost certainly be over
  // sampled.
  if ((current_process.compare(CONVERSION_PROCESS) == 0) and down_bias_conv_) {
    G4double interaction_length =
        callingProcess->GetWrappedProcess()->GetCurrentInteractionLength();

    double em_xsec_unbiased = 1. / interaction_length;

    double em_xsec_biased =
        std::max(em_xsec_unbiased + pn_xsec_unbiased_ - pn_xsec_biased_,
                 pn_xsec_unbiased_);
    static auto material_tungsten =
        simcore::g4user::ptrretrieval::getMaterial("G4_W");
    auto interaction_material = track->GetMaterial();
    if ((em_xsec_biased == pn_xsec_unbiased_) &&
        (interaction_material == material_tungsten)) {
      ldmx_log(warn) << "EM XS = PN unbiased XS! The biasing factor ("
                     << factor_ << ") is too large for particle with energy "
                     << track->GetKineticEnergy()
                     << " and material = " << interaction_material->GetName();
    }

    em_xsec_operation_->SetBiasedCrossSection(em_xsec_biased);
    em_xsec_operation_->Sample();

    return em_xsec_operation_;
  }
  return nullptr;
}

}  // namespace biasoperators
}  // namespace simcore

DECLARE_XSECBIASINGOPERATOR(simcore::biasoperators::PhotoNuclear)

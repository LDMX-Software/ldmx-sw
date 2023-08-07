
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
  /*std::cout << "[ ElectroNuclearXsecBiasingOperator ]: "
            << "Track ID: " << track->GetTrackID() << ", "
            << "Parent ID: " << track->GetParentID() << ", "
            << "Created within " << track->GetLogicalVolumeAtVertex()->GetName()
     << ", "
            << "Currently in volume " << track->GetVolume()->GetName()
            << std::endl;*/

  if (track->GetKineticEnergy() < threshold_) {
    return nullptr;
  }

  /*std::cout << "[ ElectroNuclearXsecBiasingOperator ]: "
            << "Calling process: "
            << callingProcess->GetWrappedProcess()->GetProcessName()
            << std::endl;*/

  std::string currentProcess =
      callingProcess->GetWrappedProcess()->GetProcessName();
  if (currentProcess.compare(this->getProcessToBias()) == 0) {
    G4double interactionLength =
        callingProcess->GetWrappedProcess()->GetCurrentInteractionLength();
    /*std::cout << "[ ElectroNuclearXsecBiasingOperator ]: "
              << "EN Interaction length: "
              << interactionLength << std::endl;*/

    double enXsecUnbiased = 1. / interactionLength;
    /*std::cout << "[ ElectroNuclearXsecBiasingOperator ]: Unbiased EN xsec: "
              << enXsecUnbiased << std::endl;*/

    double enXsecBiased = enXsecUnbiased * factor_;
    /*std::cout << "[ ElectroNuclearXsecBiasingOperator ]: Biased EN xsec: "
              << enXsecBiased << std::endl;*/

    return BiasedXsec(enXsecBiased);
  }
  return nullptr;
}
}  // namespace biasoperators
}  // namespace simcore

DECLARE_XSECBIASINGOPERATOR(simcore::biasoperators::ElectroNuclear)

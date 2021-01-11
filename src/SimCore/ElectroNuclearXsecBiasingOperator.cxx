/**
 * @file ElectroNuclearXsecBiasingOperator.cxx
 * @brief Geant4 Biasing Operator used to bias the occurrence of electronuclear
 *        events by modifying the cross-section.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "SimCore/ElectroNuclearXsecBiasingOperator.h"

namespace simcore {

const std::string ElectroNuclearXsecBiasingOperator::ELECTRONUCLEAR_PROCESS =
    "electronNuclear";

ElectroNuclearXsecBiasingOperator::ElectroNuclearXsecBiasingOperator(
    std::string name)
    : XsecBiasingOperator(name) {}

ElectroNuclearXsecBiasingOperator::~ElectroNuclearXsecBiasingOperator() {}

void ElectroNuclearXsecBiasingOperator::StartRun() {
  XsecBiasingOperator::StartRun();
}

G4VBiasingOperation*
ElectroNuclearXsecBiasingOperator::ProposeOccurenceBiasingOperation(
    const G4Track* track, const G4BiasingProcessInterface* callingProcess) {
  /*std::cout << "[ ElectroNuclearXsecBiasingOperator ]: "
            << "Track ID: " << track->GetTrackID() << ", "
            << "Parent ID: " << track->GetParentID() << ", "
            << "Created within " << track->GetLogicalVolumeAtVertex()->GetName()
     << ", "
            << "Currently in volume " << track->GetVolume()->GetName()
            << std::endl;*/

  if (track->GetParentID() != 0) return 0;

  // Only bias the first step taken within the volume.
  const G4Step* step = track->GetStep();
  G4StepPoint* preStepPoint = step->GetPreStepPoint();
  if (preStepPoint->GetStepStatus() != fGeomBoundary) return 0;

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

    double enXsecBiased = enXsecUnbiased * xsecFactor_;
    /*std::cout << "[ ElectroNuclearXsecBiasingOperator ]: Biased EN xsec: "
              << enXsecBiased << std::endl;*/

    xsecOperation->SetBiasedCrossSection(enXsecBiased);
    xsecOperation->Sample();

    return xsecOperation;

  } else
    return 0;
}
}  // namespace simcore

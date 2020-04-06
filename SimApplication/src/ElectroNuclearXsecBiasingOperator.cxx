/**
 * @file ElectroNuclearXsecBiasingOperator.cxx
 * @brief Geant4 Biasing Operator used to bias the occurrence of electronuclear 
 *        events by modifying the cross-section.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "SimApplication/ElectroNuclearXsecBiasingOperator.h" 

namespace ldmx { 

    const std::string ElectroNuclearXsecBiasingOperator::ELECTRONUCLEAR_PROCESS = "electronNuclear";

    
    ElectroNuclearXsecBiasingOperator::ElectroNuclearXsecBiasingOperator(std::string name) :
        XsecBiasingOperator(name) {
    }

    ElectroNuclearXsecBiasingOperator::~ElectroNuclearXsecBiasingOperator() {
    }

    void ElectroNuclearXsecBiasingOperator::StartRun() { 
        XsecBiasingOperator::StartRun(); 
    }

    G4VBiasingOperation* ElectroNuclearXsecBiasingOperator::ProposeOccurenceBiasingOperation(
            const G4Track* track, const G4BiasingProcessInterface* callingProcess) {
    
        ldmx_log(debug) 
                  << "Track ID: " << track->GetTrackID() << ", " 
                  << "Parent ID: " << track->GetParentID() << ", "
                  << "Created within " << track->GetLogicalVolumeAtVertex()->GetName() << ", " 
                  << "Currently in volume " << track->GetVolume()->GetName() ;

        if (track->GetParentID() != 0) return 0;

        // Only bias the first step taken within the volume.
        const G4Step* step = track->GetStep(); 
        G4StepPoint* preStepPoint = step->GetPreStepPoint();
        if (preStepPoint->GetStepStatus() != fGeomBoundary) return 0; 

        ldmx_log(debug)
                  << "Calling process: " 
                  << callingProcess->GetWrappedProcess()->GetProcessName() ;
        
        std::string currentProcess = callingProcess->GetWrappedProcess()->GetProcessName(); 
        if (currentProcess.compare(this->getProcessToBias()) == 0) { 
            
            G4double interactionLength = callingProcess->GetWrappedProcess()->GetCurrentInteractionLength();
            ldmx_log(debug)
                      << "EN Interaction length: " 
                      << interactionLength;

            double enXsecUnbiased = 1./interactionLength;

            double enXsecBiased = enXsecUnbiased*xsecFactor_; 

            ldmx_log(debug) << "Unbiased EN xsec: " << enXsecUnbiased << ", Biased EN xsec: " << enXsecBiased;

            xsecOperation->SetBiasedCrossSection(enXsecBiased);
            xsecOperation->Sample();

            return xsecOperation;

        } else return 0;
    }
}

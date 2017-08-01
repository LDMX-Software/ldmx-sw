/**
 * @file XsecBiasingPlugin.cxx
 * @brief Geant4 Biasing Operator used to bias the occurence of photonuclear 
 *        events by modifying the cross-section.
 * @author Omar Moreno
 *         SLAC National Accelerator Laboratory
 */

#include "Biasing/XsecBiasingOperator.h"

namespace ldmx { 

    XsecBiasingOperator::XsecBiasingOperator(std::string name) :
        G4VBiasingOperator(name) {
           messenger_ = new XsecBiasingOperatorMessenger(this);  
    }

    XsecBiasingOperator::~XsecBiasingOperator() {
    }

    void XsecBiasingOperator::StartRun() { 

        G4ProcessManager* processManager{nullptr}; 
        if (particleType_.compare("gamma") == 0) {
            std::cout << "[ XsecBiasingOperator ]: "
                      << "Applying biasing to particle type " 
                      << particleType_ 
                      << std::endl; 
            processManager = G4Gamma::GammaDefinition()->GetProcessManager();
        } else if (particleType_.compare("e-") == 0) { 
            std::cout << "[ XsecBiasingOperator ]: "
                      << "Applying biasing to particle type " 
                      << particleType_ 
                      << std::endl; 
            processManager = G4Electron::ElectronDefinition()->GetProcessManager();
        } else { 
            // Throw an exception
        }

        /*std::cout << "[ XsecBiasingOperator ]: "<< "Process: " << process_ << std::endl;*/ 
        const G4BiasingProcessSharedData* sharedData = G4BiasingProcessInterface::GetSharedData(processManager);
        if (sharedData) {
            for (size_t index = 0 ; index < (sharedData->GetPhysicsBiasingProcessInterfaces()).size(); ++index) {
                const G4BiasingProcessInterface* wrapperProcess = (sharedData->GetPhysicsBiasingProcessInterfaces())[index];
                if (wrapperProcess->GetWrappedProcess()->GetProcessName().compareTo(process_) == 0) { 
                    xsecOperation = new G4BOptnChangeCrossSection("changeXsec-" + process_);
                    break;
                } 
            }
        }
    }

    G4VBiasingOperation* XsecBiasingOperator::ProposeOccurenceBiasingOperation(
            const G4Track* track, const G4BiasingProcessInterface* callingProcess) {
        /*std::cout << "[ XsecBiasingOperator ]: " 
                    << "Calling process: " 
                    << callingProcess->GetWrappedProcess()->GetProcessName() 
                    << std::endl;*/

        if (callingProcess->GetWrappedProcess()->GetProcessName().compareTo(process_) != 0) return 0; 

        /*std::cout << "[ XsecBiasingOperator ]: "
                    << "Parent ID: " << track->GetParentID() 
                    << " Created within " << track->GetLogicalVolumeAtVertex()->GetName() 
                    << std::endl;*/
        
        if (biasIncident_ && (track->GetParentID() != 0)) {
            return 0;
        } else if (!biasAll_ && !biasIncident_ && track->GetParentID() != 1) {
            return 0;
        }

        if (track->GetKineticEnergy() < threshold_) return 0; 

        G4double interactionLength = callingProcess->GetWrappedProcess()->GetCurrentInteractionLength();
        /*std::cout << "[ XsecBiasingOperator ]: "
                    << "Interaction length: " 
                    << interactionLength << std::endl;*/

        G4double xsec = 1./interactionLength;
        /*std::cout << "[ XsecBiasingOperator ]: "
                    << "Cross-section: " 
                    << xsec << std::endl;*/

        /*std::cout << "[ XsecBiasingOperator ]: "
                    << "Cross-section x transformation factor: " 
                    << xsec*xsecFactor_ << std::endl;*/

        xsecOperation->SetBiasedCrossSection(xsec*xsecFactor_);
        xsecOperation->Sample();

        return xsecOperation;
    }
}

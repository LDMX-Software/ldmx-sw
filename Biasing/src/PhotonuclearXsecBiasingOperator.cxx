/**
 * @file PhotonuclearXsecBiasingPlugin.cxx
 * @brief Geant4 Biasing Operator used to bias the occurence of photonuclear 
 *        events by modifying the cross-section.
 * @author Omar Moreno
 *         SLAC National Accelerator Laboratory
 */

#include "Biasing/PhotonuclearXsecBiasingOperator.h"

sim::PhotonuclearXsecBiasingOperator::PhotonuclearXsecBiasingOperator(std::string name) 
    : G4VBiasingOperator(name) { 
}

sim::PhotonuclearXsecBiasingOperator::~PhotonuclearXsecBiasingOperator() {
}

void sim::PhotonuclearXsecBiasingOperator::StartRun() { 


    G4ProcessManager* processManager = G4Gamma::GammaDefinition()->GetProcessManager();
    const G4BiasingProcessSharedData* sharedData = G4BiasingProcessInterface::GetSharedData(processManager);
    if (sharedData) {
        for (size_t i = 0 ; i < (sharedData->GetPhysicsBiasingProcessInterfaces()).size(); ++i) {
            const G4BiasingProcessInterface* wrapperProcess = (sharedData->GetPhysicsBiasingProcessInterfaces())[i];
            if (wrapperProcess->GetWrappedProcess()->GetProcessName().compareTo("photonNuclear") == 0) { 
                xsecOperation = new G4BOptnChangeCrossSection("changeXsec-photoNuclear");
                break;
            } 
        }
    }
}

G4VBiasingOperation* sim::PhotonuclearXsecBiasingOperator::ProposeOccurenceBiasingOperation(const G4Track* track,
        const G4BiasingProcessInterface* callingProcess) {
    /*std::cout << "[ PhotonuclearXsecBiasingOperator ]: " 
              << "Calling process: " 
              << callingProcess->GetWrappedProcess()->GetProcessName() 
              << std::endl;*/

    if (callingProcess->GetWrappedProcess()->GetProcessName().compareTo("photonNuclear") != 0) { 
        return 0;
    }

    /*std::cout << "[ PhotonuclearXsecBiasingOperator ]: "
              << "Parent ID: " << track->GetParentID() 
              << " Created within " << track->GetLogicalVolumeAtVertex()->GetName() 
              << std::endl;*/

    if (track->GetParentID() != 1 
            || track->GetLogicalVolumeAtVertex()->GetName().compareTo(vertexVolume_) != 0) return 0;

    G4double interactionLength =  
        callingProcess->GetWrappedProcess()->GetCurrentInteractionLength();
    /*std::cout << "[ PhotonuclearXsecBiasingOperator ]: "
              << "Interaction length: " 
              << interactionLength << std::endl;*/
    
    G4double xsec = 1./interactionLength;
    /*std::cout << "[ PhotonuclearXsecBiasingOperator ]: "
              << "Cross-section: " 
              << xsec << std::endl;*/
   
    // TODO: Make this settable via a macro command 
    G4double xsecTransformation = 5000;
    /*std::cout << "[ PhotonuclearXsecBiasingOperator ]: "
              << "Cross-section x transformation factor: " 
              << xsec*xsecTransformation << std::endl;*/

    xsecOperation->SetBiasedCrossSection(xsec*xsecTransformation);
    xsecOperation->Sample();

    return xsecOperation;
}

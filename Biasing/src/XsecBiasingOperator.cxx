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
            processManager = G4Gamma::GammaDefinition()->GetProcessManager();
        } else if (particleType_.compare("e-") == 0) { 
            processManager = G4Electron::ElectronDefinition()->GetProcessManager();
        } else { 
            // Throw an exception
        }

        std::cout << "[ XsecBiasingOperator ]: " << "Biasing particles of type " 
                  << particleType_ << std::endl; 

        const G4BiasingProcessSharedData* sharedData = G4BiasingProcessInterface::GetSharedData(processManager);
        if (sharedData) {
            for (size_t index = 0 ; index < (sharedData->GetPhysicsBiasingProcessInterfaces()).size(); ++index) {
                const G4BiasingProcessInterface* wrapperProcess = (sharedData->GetPhysicsBiasingProcessInterfaces())[index];
                if (wrapperProcess->GetWrappedProcess()->GetProcessName().compareTo(this->getProcessToBias()) == 0) { 
                    xsecOperation = new G4BOptnChangeCrossSection("changeXsec-" + this->getProcessToBias());
                    break;
                } 
            }
        }
    }
}

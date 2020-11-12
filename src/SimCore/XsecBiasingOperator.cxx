/**
 * @file XsecBiasingPlugin.cxx
 * @brief Geant4 Biasing Operator used to bias the occurence of photonuclear 
 *        events by modifying the cross-section.
 * @author Omar Moreno
 *         SLAC National Accelerator Laboratory
 */

#include "SimCore/XsecBiasingOperator.h"

/*~~~~~~~~~~~~~~~*/
/*   Framework   */
/*~~~~~~~~~~~~~~~*/
#include "Framework/Exception/Exception.h" 

namespace ldmx { 

    XsecBiasingOperator::XsecBiasingOperator(std::string name) :
        G4VBiasingOperator(name) {
    }

    XsecBiasingOperator::~XsecBiasingOperator() {
    }

    void XsecBiasingOperator::StartRun() { 

        if (particleType_.compare("gamma") == 0) {
            processManager_ = G4Gamma::GammaDefinition()->GetProcessManager();
        } else if (particleType_.compare("e-") == 0) { 
            processManager_ = G4Electron::ElectronDefinition()->GetProcessManager();
        } else {
            EXCEPTION_RAISE("BiasingException", "Invalid particle type"); 
        }

        std::cout << "[ XsecBiasingOperator ]: Biasing particles of type " 
                  << particleType_ << std::endl; 

        if (processIsBiased(this->getProcessToBias())) { 
            xsecOperation = new G4BOptnChangeCrossSection("changeXsec-" + this->getProcessToBias());
        } else { 
            EXCEPTION_RAISE(
                    "BiasSetup",
                    this->getProcessToBias()+" is not found in list of biased processes!"
                    );
        }
    }

    bool XsecBiasingOperator::processIsBiased(std::string process) { 
        
        // Loop over all processes and check if the given process is being 
        // biased.
        const G4BiasingProcessSharedData* sharedData 
            = G4BiasingProcessInterface::GetSharedData(processManager_);
        if (sharedData) {
            for (size_t iprocess = 0 ; 
                    iprocess < (sharedData->GetPhysicsBiasingProcessInterfaces()).size(); ++iprocess) {
                
                const G4BiasingProcessInterface* wrapperProcess
                    = (sharedData->GetPhysicsBiasingProcessInterfaces())[iprocess];

                if (wrapperProcess->GetWrappedProcess()->GetProcessName().compareTo(process) == 0) {
                    return true; 
                } 
            }
        }
        return false;
    }
}

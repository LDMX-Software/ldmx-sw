/**
 * @file XsecBiasingPlugin.cxx
 * @brief Geant4 Biasing Operator used to bias the occurence of photonuclear 
 *        events by modifying the cross-section.
 * @author Omar Moreno
 *         SLAC National Accelerator Laboratory
 */

#include "SimCore/PhotoNuclearXsecBiasingOperator.h"

namespace ldmx { 

    const std::string PhotoNuclearXsecBiasingOperator::PHOTONUCLEAR_PROCESS = "photonNuclear"; 
    
    const std::string PhotoNuclearXsecBiasingOperator::CONVERSION_PROCESS = "conv";  

    PhotoNuclearXsecBiasingOperator::PhotoNuclearXsecBiasingOperator(std::string name) :
        XsecBiasingOperator(name) {
    }

    PhotoNuclearXsecBiasingOperator::~PhotoNuclearXsecBiasingOperator() {
    }

    void PhotoNuclearXsecBiasingOperator::StartRun() { 
     
        XsecBiasingOperator::StartRun(); 

        if (processIsBiased(CONVERSION_PROCESS)) { 
            emXsecOperation = new G4BOptnChangeCrossSection("changeXsec-conv");
        } else { 
            // Throw an exception
        }
    }

    G4VBiasingOperation* PhotoNuclearXsecBiasingOperator::ProposeOccurenceBiasingOperation(
            const G4Track* track, const G4BiasingProcessInterface* callingProcess) {
    
        /*std::cout << "[ PhotoNuclearXsecBiasingOperator ]: " 
                  << "Kinetic energy: " << track->GetKineticEnergy() 
                  << " MeV" << std::endl;*/

        if (track->GetKineticEnergy() < XsecBiasingOperator::threshold_) return 0; 

        /*std::cout << "[ PhotoNuclearXsecBiasingOperator ]: " 
                  << "Calling process: " 
                  << callingProcess->GetWrappedProcess()->GetProcessName() 
                  << std::endl;*/

        std::string currentProcess = callingProcess->GetWrappedProcess()->GetProcessName(); 
        if (currentProcess.compare(this->getProcessToBias()) == 0) { 
            
            G4double interactionLength = callingProcess->GetWrappedProcess()->GetCurrentInteractionLength();
            /*std::cout << "[ PhotoNuclearXsecBiasingOperator ]: "
                      << "PN Interaction length: " 
                      << interactionLength << std::endl;*/

            pnXsecUnbiased_ = 1./interactionLength;
            /*std::cout << "[ PhotoNuclearXsecBiasingOperator ]: Unbiased PN xsec: "
                      << pnXsecUnbiased_ << std::endl;*/

            pnXsecBiased_ = pnXsecUnbiased_*xsecFactor_; 
            /*std::cout << "[ PhotoNuclearXsecBiasingOperator ]: Biased PN xsec: "
                      << pnXsecBiased_ << std::endl;*/

            xsecOperation->SetBiasedCrossSection(pnXsecBiased_);
            xsecOperation->Sample();

            return xsecOperation;
    
        } else if ((currentProcess.compare(CONVERSION_PROCESS) == 0) && biasDownEM_) { 
            
            G4double interactionLength = callingProcess->GetWrappedProcess()->GetCurrentInteractionLength();
            /*std::cout << "[ PhotoNuclearXsecBiasingOperator ]: "
                      << "EM Interaction length: " 
                      << interactionLength << std::endl;*/

            double emXsecUnbiased = 1./interactionLength; 
            /*std::cout << "[ PhotoNuclearXsecBiasingOperator ]: Unbiased EM xsec: "
                      << emXsecUnbiased << std::endl;*/

            double emXsecBiased = std::max(emXsecUnbiased + pnXsecUnbiased_ - pnXsecBiased_, pnXsecUnbiased_); 
            if (emXsecBiased == pnXsecUnbiased_) { 
                G4cout << "[ PhotoNuclearXsecBiasingOperator ]: [ WARNING ]: "
                       << "Biasing factor is too large." << std::endl; 
            } 
            /*std::cout << "[ PhotoNuclearXsecBiasingOperator ]: Biased EM xsec: "
                      << emXsecBiased << std::endl;*/

            emXsecOperation->SetBiasedCrossSection(emXsecBiased);
            emXsecOperation->Sample();

            return emXsecOperation;
        
        } else return 0; 

        // TODO: These should be pulled out to their own operator ... 
        /*if (XsecBiasingOperator::biasIncident_ && (track->GetParentID() != 0)) {
            return 0;
        } else if (!XsecBiasingOperator::biasAll_ && !XsecBiasingOperator::biasIncident_ && track->GetParentID() != 1) {
            return 0;
        }*/
    }
}

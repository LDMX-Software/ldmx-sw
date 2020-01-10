/**
 * @file DarkBremXsecBiasingPlugin.cxx
 * @brief Geant4 Biasing Operator used to bias the occurence of dark brem
 *        events by modifying the cross-section.
 * @author Michael Revering, University of Minnesota
 */

#include "Biasing/DarkBremXsecBiasingOperator.h"

//------------//
//   Geant4   //
//------------//
#include "G4BiasingProcessInterface.hh"
#include "G4Track.hh"
#include "G4VBiasingOperator.hh"

namespace ldmx { 

    const std::string DarkBremXsecBiasingOperator::DARKBREM_PROCESS = "eDBrem"; 
    
    DarkBremXsecBiasingOperator::DarkBremXsecBiasingOperator(std::string name) :
        XsecBiasingOperator(name) {
    }

    DarkBremXsecBiasingOperator::~DarkBremXsecBiasingOperator() {
    }

    void DarkBremXsecBiasingOperator::StartRun() { 
        XsecBiasingOperator::StartRun(); 
    }

    G4VBiasingOperation* DarkBremXsecBiasingOperator::ProposeOccurenceBiasingOperation(
            const G4Track* track, const G4BiasingProcessInterface* callingProcess) {
    
        /*std::cout << "[ DarkBremXsecBiasingOperator ]: "
                  << "Parent ID: " << track->GetParentID() 
                  << " Created within " << track->GetLogicalVolumeAtVertex()->GetName() 
                  << std::endl;*/

        //only bias primary particle
        if (track->GetParentID() != 0) return 0; 

        /*std::cout << "[ DarkBremXsecBiasingOperator ]: " 
                  << "Kinetic energy: " << track->GetKineticEnergy() 
                  << " MeV" << std::endl;*/

        //only bias primary particles above the minimum energy
        if (track->GetKineticEnergy() < XsecBiasingOperator::threshold_) return 0; 

        /*std::cout << "[ DarkBremXsecBiasingOperator ]: " 
                  << "Calling process: " 
                  << callingProcess->GetWrappedProcess()->GetProcessName() 
                  << std::endl;*/

        std::string currentProcess = callingProcess->GetWrappedProcess()->GetProcessName(); 
        if (currentProcess.compare(this->getProcessToBias()) == 0) { 
            //only bias the process that we want to DARKBREM_PROCESS
            
            G4double interactionLength = callingProcess->GetWrappedProcess()->GetCurrentInteractionLength();
            /*std::cout << "[ DarkBremXsecBiasingOperator ]: "
                      << "PN Interaction length: " 
                      << interactionLength << std::endl;*/

            dbXsecUnbiased_ = 1./interactionLength;
            std::cout << "[ DarkBremXsecBiasingOperator ]: Unbiased DBrem xsec: "
                      << dbXsecUnbiased_ << std::endl;

            dbXsecBiased_ = dbXsecUnbiased_*xsecFactor_; 
            std::cout << "[ DarkBremXsecBiasingOperator ]: Biased DBrem xsec: "
                      << dbXsecBiased_ << std::endl;

            //xsecOperation is a protected member variable of XsecBiasingOperator
            //  it is set in XsecBiasingOperator::StartRun()
            xsecOperation->SetBiasedCrossSection(dbXsecBiased_);
            xsecOperation->Sample();
            return xsecOperation;
    
        } else return 0; 

        // TODO: These should be pulled out to their own operator ... 
        /*if (XsecBiasingOperator::biasIncident_ && (track->GetParentID() != 0)) {
            return 0;
        } else if (!XsecBiasingOperator::biasAll_ && !XsecBiasingOperator::biasIncident_ && track->GetParentID() != 1) {
            return 0;
        }*/
    }
}

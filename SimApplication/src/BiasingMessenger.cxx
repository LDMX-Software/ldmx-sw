/**
 * @file BiasingMessenger.cxx
 * @brief Messenger used to pass physics biasing parameters.
 * @author Omar Moreno
 *         SLAC National Accelerator Laboratory
 */

#include "SimApplication/BiasingMessenger.h"

sim::BiasingMessenger::BiasingMessenger(G4RunManager* runManager) { 
    
    runManager_ = static_cast<RunManager*>(runManager); 

    biasingDir_->SetGuidance("LDMX physics biasing commands.");
    
    enableBiasingCmd_->AvailableForStates(G4ApplicationState::G4State_PreInit);
    enableBiasingCmd_->SetGuidance("Enable physics biasing.");

    particleTypeCmd_->SetParameterName("type", true); 
    particleTypeCmd_->AvailableForStates(G4ApplicationState::G4State_PreInit); 
    particleTypeCmd_->SetGuidance("The particle type to bias.");
}

sim::BiasingMessenger::~BiasingMessenger() {
    delete enableBiasingCmd_;
    delete biasingDir_; 
}

void sim::BiasingMessenger::SetNewValue(G4UIcommand* command, G4String newValues) { 

    if (command == enableBiasingCmd_) { 
       runManager_->enableBiasing();  
    } else if (command == particleTypeCmd_) { 
       runManager_->setParticleTypeToBias(newValues);  
    }
}

/**
 * @file BiasingMessenger.cxx
 * @brief Messenger used to pass physics biasing parameters.
 * @author Omar Moreno
 *         SLAC National Accelerator Laboratory
 */

#include "Biasing/BiasingMessenger.h"

ldmx::BiasingMessenger::BiasingMessenger() { 
    
    biasingDir_->SetGuidance("LDMX physics biasing commands.");
    
    enableBiasingCmd_->AvailableForStates(G4ApplicationState::G4State_PreInit);
    enableBiasingCmd_->SetGuidance("Enable physics biasing.");

    particleTypeCmd_->SetParameterName("type", true); 
    particleTypeCmd_->AvailableForStates(G4ApplicationState::G4State_PreInit); 
    particleTypeCmd_->SetGuidance("The particle type to bias.");
}

ldmx::BiasingMessenger::~BiasingMessenger() {
    delete enableBiasingCmd_;
    delete biasingDir_; 
}

void ldmx::BiasingMessenger::SetNewValue(G4UIcommand* command, G4String newValues) { 

    if (command == enableBiasingCmd_) biasingEnabled_ = true; 
    else if (command == particleTypeCmd_) particleType_ = newValues;  
    else if (command == processCmd_)      process_ = newValues;
    else if (command == volumeCmd_)       volume_ = newValues; 
    else if (command == xsecTransCmd_)    xsecTrans_ = G4UIcommand::ConvertToDouble(newValues); 
}

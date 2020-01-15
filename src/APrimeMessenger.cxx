/**
 * @file APrimeMessnger.cxx
 * @brief Messenger used to pass parameters used to setup a parallel world.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "SimApplication/APrimeMessenger.h"

namespace ldmx { 
   
    APrimeMessenger::APrimeMessenger(RunManager* runManager) : runManager_(runManager) {
        
        aprimeDir_ = new G4UIdirectory("/ldmx/aprime/");
        aprimeDir_->SetGuidance("UI commands specify A' sim details.");
        
        massCmd_ = new G4UIcmdWithADoubleAndUnit( "/ldmx/aprime/mass" , this );
        massCmd_->SetGuidance("Mass (with units) of the A'");
        massCmd_->AvailableForStates(G4ApplicationState::G4State_PreInit); 
    }

    APrimeMessenger::~APrimeMessenger() { 
        delete massCmd_;
        delete aprimeDir_; 
    }

    void APrimeMessenger::SetNewValue(G4UIcommand* command, G4String newValues) { 
        
        if (command == massCmd_ ) 
            runManager_->setAPrimeMass( massCmd_->GetNewDoubleValue( newValues ) );

    }
}

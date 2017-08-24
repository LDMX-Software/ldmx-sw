/**
 * @file ParallelWorldMessnger.cxx
 * @brief Messenger used to pass parameters used to setup a parallel world.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "SimApplication/ParallelWorldMessenger.h"

namespace ldmx { 
   
    ParallelWorldMessenger::ParallelWorldMessenger(RunManager* runManager) :
   runManager_(runManager) {
        
        pwDir_->SetGuidance("UI commands specific to parallel worlds.");
        
        enablePWCmd_->SetGuidance("Enable the use of a parallel world.");
        enablePWCmd_->AvailableForStates(G4ApplicationState::G4State_PreInit);
        
        readCmd_->SetGuidance("The GDML file containing the description of the parallel world.");
        readCmd_->AvailableForStates(G4ApplicationState::G4State_PreInit); 
    }

    ParallelWorldMessenger::~ParallelWorldMessenger() { 
        delete enablePWCmd_;
        delete readCmd_;
        delete pwDir_; 
    }

    void ParallelWorldMessenger::SetNewValue(G4UIcommand* command, G4String newValues) { 
        
        if (command == enablePWCmd_) runManager_->enableParallelWorld(true);
        else if (command == readCmd_) runManager_->setParallelWorldPath(newValues); 
    }
}

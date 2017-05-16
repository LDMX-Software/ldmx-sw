/**
 * @file ParallelWorldMessnger.cxx
 * @brief Messenger used to pass parameters used to setup a parallel world.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "SimApplication/ParallelWorldMessenger.h"

namespace ldmx { 
   
    /** Flag indicating if a parallel world should be loaded. */
    bool ParallelWorldMessenger::enableParallelWorld_{false};

    /** Path to GDML file containing the detector description. */
    std::string ParallelWorldMessenger::gdmlPath_{""}; 

    ParallelWorldMessenger::ParallelWorldMessenger() {
        
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
        
        if (command == enablePWCmd_) enableParallelWorld_ = true;
        else if (command == readCmd_) gdmlPath_ = newValues; 
    }
}

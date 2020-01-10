/**
 * @file TargetDarkBremFilterMessenger.cxx
 * @brief Messenger for setting parameters on TargetDarkBremFilter.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "Biasing/TargetDarkBremFilterMessenger.h"
#include <iostream>

namespace ldmx { 
    
    TargetDarkBremFilterMessenger::TargetDarkBremFilterMessenger(TargetDarkBremFilter* filter) :
        UserActionPluginMessenger(filter), filter_(filter) {

            volumeCmd_ = new G4UIcmdWithAString{std::string(getPath() + "volume").c_str(), this};
            volumeCmd_->AvailableForStates(G4ApplicationState::G4State_PreInit,
                                           G4ApplicationState::G4State_Idle);
            volumeCmd_->SetGuidance("Volume to apply the filter to.");     
    }

    TargetDarkBremFilterMessenger::~TargetDarkBremFilterMessenger() {
        delete volumeCmd_; 
    }

    void TargetDarkBremFilterMessenger::SetNewValue(G4UIcommand* command, G4String newValue) {
        
        // Handles verbose command.
        UserActionPluginMessenger::SetNewValue(command, newValue);

        if (command == volumeCmd_) {
            filter_->setVolume(newValue);
        }
    }
}

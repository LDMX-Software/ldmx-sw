/**
 * @file TargetENProcessFilterMessenger.cxx
 * @brief Messenger for setting parameters on TargetENProcessFilter.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "Biasing/TargetENProcessFilterMessenger.h"
#include <iostream>

namespace ldmx { 
    
    TargetENProcessFilterMessenger::TargetENProcessFilterMessenger(TargetENProcessFilter* filter) :
        UserActionPluginMessenger(filter), filter_(filter) {

            recoilEnergyThresholdCmd_ = new G4UIcmdWithAString{std::string(getPath() + "recoil_threshold").c_str(), this};
            recoilEnergyThresholdCmd_->AvailableForStates(G4ApplicationState::G4State_PreInit,
                                              G4ApplicationState::G4State_Idle);
            recoilEnergyThresholdCmd_->SetGuidance("Energy threshold that the recoil electron must not exceed."); 

            volumeCmd_ = new G4UIcmdWithAString{std::string(getPath() + "volume").c_str(), this};
            volumeCmd_->AvailableForStates(G4ApplicationState::G4State_PreInit,
                                           G4ApplicationState::G4State_Idle);
            volumeCmd_->SetGuidance("Volume to apply the filter to.");     
    }

    TargetENProcessFilterMessenger::~TargetENProcessFilterMessenger() {
        delete recoilEnergyThresholdCmd_;
        delete volumeCmd_; 
    }

    void TargetENProcessFilterMessenger::SetNewValue(G4UIcommand* command, G4String newValue) {
        
        // Handles verbose command.
        UserActionPluginMessenger::SetNewValue(command, newValue);

        if (command == volumeCmd_) {
            filter_->setVolume(newValue);
        } else if (command == recoilEnergyThresholdCmd_)  {
            filter_->setRecoilEnergyThreshold(G4UIcommand::ConvertToDouble(newValue));
        } 
    }
}

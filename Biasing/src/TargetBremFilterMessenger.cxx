/**
 * @file TargetBremFilterMessenger.cxx
 * @brief Messenger for setting parameters on TargetBremFilter.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "Biasing/TargetBremFilterMessenger.h"
#include <iostream>

namespace ldmx { 
    
    TargetBremFilterMessenger::TargetBremFilterMessenger(TargetBremFilter* filter) :
        UserActionPluginMessenger(filter), filter_(filter) {

            bremEnergyThresholdCmd_ = new G4UIcmdWithAString{std::string(getPath() + "brem_threshold").c_str(), this};
            bremEnergyThresholdCmd_->AvailableForStates(G4ApplicationState::G4State_PreInit,
                                              G4ApplicationState::G4State_Idle);
            bremEnergyThresholdCmd_->SetGuidance("Minium energy that the brem electron should have."); 

            killRecoilCmd_ 
                = new G4UIcmdWithoutParameter{std::string(getPath() + "kill_recoil").c_str(), this};
            killRecoilCmd_->AvailableForStates(G4ApplicationState::G4State_PreInit,
                    G4ApplicationState::G4State_Idle);
            killRecoilCmd_->SetGuidance("Enable killing of the electron track that produces the brem."); 


            recoilEnergyThresholdCmd_ = new G4UIcmdWithAString{std::string(getPath() + "recoil_threshold").c_str(), this};
            recoilEnergyThresholdCmd_->AvailableForStates(G4ApplicationState::G4State_PreInit,
                                              G4ApplicationState::G4State_Idle);
            recoilEnergyThresholdCmd_->SetGuidance("Energy threshold that the recoil electron must not exceed."); 

            volumeCmd_ = new G4UIcmdWithAString{std::string(getPath() + "volume").c_str(), this};
            volumeCmd_->AvailableForStates(G4ApplicationState::G4State_PreInit,
                                           G4ApplicationState::G4State_Idle);
            volumeCmd_->SetGuidance("Volume to apply the filter to.");     
    }

    TargetBremFilterMessenger::~TargetBremFilterMessenger() {
        delete bremEnergyThresholdCmd_;
        delete killRecoilCmd_;
        delete recoilEnergyThresholdCmd_;
        delete volumeCmd_; 
    }

    void TargetBremFilterMessenger::SetNewValue(G4UIcommand* command, G4String newValue) {
        
        if (command == killRecoilCmd_) { 
            filter_->setKillRecoilElectron(true); 
        } else if (command == volumeCmd_) {
            filter_->setVolume(newValue);
        } else if (command == recoilEnergyThresholdCmd_)  {
            filter_->setRecoilEnergyThreshold(G4UIcommand::ConvertToDouble(newValue));
        } else if (command == bremEnergyThresholdCmd_) {
            filter_->setBremEnergyThreshold(G4UIcommand::ConvertToDouble(newValue));
        }
    }
}

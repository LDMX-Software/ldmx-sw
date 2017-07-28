/**
 * @file SimpleProcessFilterMessenger.cxx
 * @brief Messenger for setting parameters on SimpleProcessFilter.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "Biasing/SimpleProcessFilterMessenger.h"
#include <iostream>

namespace ldmx { 
    
    SimpleProcessFilterMessenger::SimpleProcessFilterMessenger(SimpleProcessFilter* filter) :
        UserActionPluginMessenger(filter), filter_(filter) {

            parentIDCmd_ = new G4UIcmdWithAString{std::string(getPath() + "parent_id").c_str(), this};
            parentIDCmd_->AvailableForStates(G4ApplicationState::G4State_PreInit,
                                              G4ApplicationState::G4State_Idle);
            parentIDCmd_->SetGuidance("Parent ID of the particle to which the filter will be applied to."); 

            trackIDCmd_ = new G4UIcmdWithAString{std::string(getPath() + "track_id").c_str(), this};
            trackIDCmd_->AvailableForStates(G4ApplicationState::G4State_PreInit,
                                              G4ApplicationState::G4State_Idle);
            trackIDCmd_->SetGuidance("Track ID of the particle to which the filter will be applied to."); 

            pdgIDCmd_ = new G4UIcmdWithAString{std::string(getPath() + "pdg_id").c_str(), this};
            pdgIDCmd_->AvailableForStates(G4ApplicationState::G4State_PreInit,
                                              G4ApplicationState::G4State_Idle);
            pdgIDCmd_->SetGuidance("PDG ID of the particle to which the filter will be applied to."); 

            volumeCmd_ = new G4UIcmdWithAString{std::string(getPath() + "volume").c_str(), this};
            volumeCmd_->AvailableForStates(G4ApplicationState::G4State_PreInit,
                                           G4ApplicationState::G4State_Idle);
            volumeCmd_->SetGuidance("Volume to apply the filter to.");     
    }

    SimpleProcessFilterMessenger::~SimpleProcessFilterMessenger() {
        delete parentIDCmd_;
        delete trackIDCmd_;
        delete pdgIDCmd_;
        delete volumeCmd_; 
    }

    void SimpleProcessFilterMessenger::SetNewValue(G4UIcommand* command, G4String newValue) {
        
        if (command == volumeCmd_) {
            filter_->setVolume(newValue);
        } else if (command == parentIDCmd_)  {
            filter_->setParentID(G4UIcommand::ConvertToInt(newValue));
        } else if (command == trackIDCmd_)  {
            filter_->setTrackID(G4UIcommand::ConvertToInt(newValue));
        } else if (command == pdgIDCmd_)  {
            filter_->setPdgID(G4UIcommand::ConvertToInt(newValue));
        }
    }
}

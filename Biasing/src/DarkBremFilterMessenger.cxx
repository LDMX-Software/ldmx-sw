/**
 * @file DarkBremFilterMessenger.cxx
 * @brief Messenger for setting parameters on DarkBremFilter.
 * @author Michael Revering, University of Minnesota
 * @author Tom Eichlersmith, University of Minnesota
 */

#include "Biasing/DarkBremFilterMessenger.h"

//-------------//
//   ldmx-sw   //
//-------------//
#include "Biasing/DarkBremFilter.h"

//-------------//
//   Geant4    //
//-------------//
#include "G4UIcmdWithAString.hh"

namespace ldmx { 
    
    DarkBremFilterMessenger::DarkBremFilterMessenger(DarkBremFilter* filter) :
        UserActionPluginMessenger(filter), filter_(filter) {

            volumeCmd_ = new G4UIcmdWithAString{std::string(getPath() + "volume").c_str(), this};
            volumeCmd_->AvailableForStates(G4ApplicationState::G4State_PreInit,
                                           G4ApplicationState::G4State_Idle);
            volumeCmd_->SetGuidance("Volume to apply the filter to.");     
    }

    DarkBremFilterMessenger::~DarkBremFilterMessenger() {
        delete volumeCmd_; 
    }

    void DarkBremFilterMessenger::SetNewValue(G4UIcommand* command, G4String newValue) {
        
        // Handles verbose command.
        UserActionPluginMessenger::SetNewValue(command, newValue);

        if (command == volumeCmd_) {
            filter_->setVolume(newValue);
        }
    }
}

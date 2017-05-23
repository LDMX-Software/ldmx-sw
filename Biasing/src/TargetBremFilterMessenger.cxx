/**
 * @file TargetBremFilterMessenger.cxx
 * @brief Messenger for setting parameters on TargetBremFilter.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "Biasing/TargetBremFilterMessenger.h"

namespace ldmx { 
    
    TargetBremFilterMessenger::TargetBremFilterMessenger(TargetBremFilter* filter) :
        UserActionPluginMessenger(filter), filter_(filter) {

            killElectronCmd_ 
                = new G4UIcmdWithoutParameter{std::string(getPath() + "kill_electron").c_str(), this};
            killElectronCmd_->AvailableForStates(G4ApplicationState::G4State_PreInit);
            killElectronCmd_->SetGuidance("Enable killing of the electron track that produces the brem."); 
    }

    TargetBremFilterMessenger::~TargetBremFilterMessenger() {
        delete killElectronCmd_;
    }

    void TargetBremFilterMessenger::SetNewValue(G4UIcommand* command, G4String newValue) {
        
        if (command == killElectronCmd_) killElectron_ = true; 
    }
}

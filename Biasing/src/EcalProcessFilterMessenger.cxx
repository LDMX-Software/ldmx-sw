/**
 * @file EcalProcessFilterMessenger.cxx
 * @brief Messenger for setting parameters on EcalProcessFilter.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "Biasing/EcalProcessFilterMessenger.h"
#include <iostream>

namespace ldmx { 
    
    EcalProcessFilterMessenger::EcalProcessFilterMessenger(EcalProcessFilter* filter) :
        UserActionPluginMessenger(filter), filter_(filter) {

            boundCmd_ 
                = new G4UIcmdWithAString{std::string(getPath() + "bound_volume").c_str(), this};
            boundCmd_->AvailableForStates(G4ApplicationState::G4State_PreInit,
                    G4ApplicationState::G4State_Idle);
            boundCmd_->SetGuidance("Bound a particle to the given volume."); 


            volumeCmd_ = new G4UIcmdWithAString{std::string(getPath() + "volume").c_str(), this};
            volumeCmd_->AvailableForStates(G4ApplicationState::G4State_PreInit,
                                           G4ApplicationState::G4State_Idle);
            volumeCmd_->SetGuidance("Volume to apply the filter to. Note that multiple volumes may be added.");     
    }

    EcalProcessFilterMessenger::~EcalProcessFilterMessenger() {
        delete volumeCmd_; 
    }

    void EcalProcessFilterMessenger::SetNewValue(G4UIcommand* command, G4String newValue) {

        // Handles verbose command.
        UserActionPluginMessenger::SetNewValue(command, newValue);

        if (command == volumeCmd_) filter_->addVolume(newValue);
        else if (command == boundCmd_) filter_->addBoundingVolume(newValue); 
    }
}

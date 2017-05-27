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

            volumeCmd_ = new G4UIcmdWithAString{std::string(getPath() + "volume").c_str(), this};
            volumeCmd_->AvailableForStates(G4ApplicationState::G4State_PreInit,
                                           G4ApplicationState::G4State_Idle);
            volumeCmd_->SetGuidance("Volume to apply the filter to. Note that multiple volumes may be added.");     
    }

    EcalProcessFilterMessenger::~EcalProcessFilterMessenger() {
        delete volumeCmd_; 
    }

    void EcalProcessFilterMessenger::SetNewValue(G4UIcommand* command, G4String newValue) {
        if (command == volumeCmd_) filter_->addVolume(newValue); 
    }
}

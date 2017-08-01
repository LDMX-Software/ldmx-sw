/**
 * @file BiasingMessenger.cxx
 * @brief Messenger used to pass physics biasing parameters.
 * @author Omar Moreno
 *         SLAC National Accelerator Laboratory
 */

#include "Biasing/BiasingMessenger.h"

namespace ldmx { 

    bool BiasingMessenger::biasingEnabled_{false}; 

    double BiasingMessenger::eventWeight_{1}; 

    std::string BiasingMessenger::particleType_{"gamma"};

    std::string BiasingMessenger::process_{"photonNuclear"};

    double BiasingMessenger::threshold_{0};

    std::string BiasingMessenger::volume_{"target"};

    BiasingMessenger::BiasingMessenger() { 

        biasingDir_->SetGuidance("LDMX physics biasing commands.");

        enableBiasingCmd_->AvailableForStates(G4ApplicationState::G4State_PreInit);
        enableBiasingCmd_->SetGuidance("Enable physics biasing.");

        particleTypeCmd_->SetParameterName("type", true); 
        particleTypeCmd_->AvailableForStates(G4ApplicationState::G4State_PreInit); 
        particleTypeCmd_->SetGuidance("The particle type to bias.");
    }

    BiasingMessenger::~BiasingMessenger() {
        delete enableBiasingCmd_;
        delete biasingDir_; 
    }

    void BiasingMessenger::SetNewValue(G4UIcommand* command, G4String newValues) { 

        if (command == enableBiasingCmd_) biasingEnabled_ = true; 
        else if (command == particleTypeCmd_) particleType_ = newValues;  
        else if (command == processCmd_) process_ = newValues;
        else if (command == thresholdCmd_)threshold_ = G4UIcommand::ConvertToDouble(newValues);
        else if (command == volumeCmd_) volume_ = newValues;
    }

}

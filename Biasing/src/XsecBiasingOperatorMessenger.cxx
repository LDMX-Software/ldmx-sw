/**
 * @file XsecBiasingOperatorMessenger.cxx
 * @brief Messenger for setting parameters on XsecBiasingOperator.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "Biasing/XsecBiasingOperatorMessenger.h"

namespace ldmx { 
    
    XsecBiasingOperatorMessenger::XsecBiasingOperatorMessenger(XsecBiasingOperator* oprt) :
        operator_(oprt) {
    
        biasingDir_ = new G4UIdirectory{"/ldmx/biasing/xsec/"};

        biasAllCmd_ = new G4UIcmdWithoutParameter{"/ldmx/biasing/xsec/bias_all", this};
        biasAllCmd_->AvailableForStates(G4ApplicationState::G4State_PreInit,
                                        G4ApplicationState::G4State_Idle);
        biasAllCmd_->SetGuidance("Bias all particles of the given type."); 

        biasIncidentCmd_ = new G4UIcmdWithoutParameter{"/ldmx/biasing/xsec/bias_incident", this};
        biasIncidentCmd_->AvailableForStates(G4ApplicationState::G4State_PreInit,
                                        G4ApplicationState::G4State_Idle);
        biasIncidentCmd_->SetGuidance("Bias only the incident particle."); 

        particleTypeCmd_ = new G4UIcmdWithAString{"/ldmx/biasing/xsec/particle", this};
        particleTypeCmd_->AvailableForStates(G4ApplicationState::G4State_PreInit,
                                             G4ApplicationState::G4State_Idle);
        particleTypeCmd_->SetGuidance("Set the particle type to bias."); 

        thresholdCmd_ = new G4UIcmdWithAString{"/ldmx/biasing/xsec/threshold", this};
        thresholdCmd_->AvailableForStates(G4ApplicationState::G4State_PreInit,
                                          G4ApplicationState::G4State_Idle);
        thresholdCmd_->SetGuidance("Set the minimum energy required to bias a particle."); 
        
        xsecTransCmd_ = new G4UIcmdWithAString{"/ldmx/biasing/xsec/factor", this};
        xsecTransCmd_->AvailableForStates(G4ApplicationState::G4State_PreInit,  
                                          G4ApplicationState::G4State_Idle);
        xsecTransCmd_->SetGuidance("Set the factor by which the xsec will increase."); 
    }

    XsecBiasingOperatorMessenger::~XsecBiasingOperatorMessenger() {
        delete biasingDir_;
        delete biasAllCmd_;
        delete biasIncidentCmd_;  
        delete particleTypeCmd_;
        delete thresholdCmd_;
        delete xsecTransCmd_;
    }

    void XsecBiasingOperatorMessenger::SetNewValue(G4UIcommand* command, G4String newValue) {
        
        if (command == biasAllCmd_) { 
            operator_->biasAll(); 
        } else if (command == biasIncidentCmd_) { 
            operator_->biasIncident(); 
        } else if (command == particleTypeCmd_)  {
            operator_->setParticleType(newValue); 
        } else if (command == thresholdCmd_) { 
            operator_->setThreshold(G4UIcommand::ConvertToDouble(newValue)); 
        } else if (command == xsecTransCmd_) { 
            operator_->setXsecFactor(G4UIcommand::ConvertToDouble(newValue));
        }
    }
}

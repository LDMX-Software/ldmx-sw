#include "SimApplication/PrimaryGeneratorMessenger.h"

// LDMX
#include "SimApplication/LHEPrimaryGenerator.h"

namespace sim {

PrimaryGeneratorMessenger::PrimaryGeneratorMessenger(PrimaryGeneratorAction* thePrimaryGeneratorAction) :
    primaryGeneratorAction_(thePrimaryGeneratorAction) {

    lheDir_ = new G4UIdirectory("/ldmx/generators/lhe/");
    lheDir_->SetGuidance("Commands for LHE event generation");

    lheOpenCmd_ = new G4UIcommand("/ldmx/generators/lhe/open", this);
    G4UIparameter* filename = new G4UIparameter("filename", 's', true);
    lheOpenCmd_->SetParameter(filename);
    lheOpenCmd_->AvailableForStates(G4ApplicationState::G4State_PreInit, G4ApplicationState::G4State_Idle);
}

PrimaryGeneratorMessenger::~PrimaryGeneratorMessenger() {

}

void PrimaryGeneratorMessenger::SetNewValue(G4UIcommand* command, G4String newValues) {
    if (command == lheOpenCmd_) {
        primaryGeneratorAction_->setPrimaryGenerator(
                new LHEPrimaryGenerator(new LHEReader(newValues)));
    }
}

}

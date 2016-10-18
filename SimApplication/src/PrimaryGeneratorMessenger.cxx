#include "SimApplication/PrimaryGeneratorMessenger.h"

// LDMX
#include "SimApplication/LHEPrimaryGenerator.h"

namespace sim {

PrimaryGeneratorMessenger::PrimaryGeneratorMessenger(PrimaryGeneratorAction* thePrimaryGeneratorAction) :
    primaryGeneratorAction(thePrimaryGeneratorAction) {

    lheDir = new G4UIdirectory("/ldmx/generators/lhe/");
    lheDir->SetGuidance("Commands for LHE event generation");

    lheOpenCmd = new G4UIcommand("/ldmx/generators/lhe/open", this);
    G4UIparameter* filename = new G4UIparameter("filename", 's', true);
    lheOpenCmd->SetParameter(filename);
    lheOpenCmd->AvailableForStates(G4ApplicationState::G4State_PreInit, G4ApplicationState::G4State_Idle);
}

PrimaryGeneratorMessenger::~PrimaryGeneratorMessenger() {

}

void PrimaryGeneratorMessenger::SetNewValue(G4UIcommand* command, G4String newValues) {
    if (command == lheOpenCmd) {
        primaryGeneratorAction->setPrimaryGenerator(
                new LHEPrimaryGenerator(new LHEReader(newValues)));
    }
}

}

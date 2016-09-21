#include "SimApplication/SimApplicationMessenger.h"

// LDMX
#include "Event/RootEventWriter.h"

// Geant4
#include "G4ApplicationState.hh"

SimApplicationMessenger::SimApplicationMessenger() {

    ldmxDir = new G4UIdirectory("/ldmx/");
    ldmxDir->SetGuidance("LDMX Simulation Application commands");

    persistencyDir = new G4UIdirectory("/ldmx/persistency/");
    persistencyDir->SetGuidance("Persistency commands");

    rootDir = new G4UIdirectory("/ldmx/persistency/root/");
    rootDir->SetGuidance("ROOT persistency commands");

    rootFileCmd = new G4UIcommand("/ldmx/persistency/root/filename", this);
    G4UIparameter* filename = new G4UIparameter("filename", 's', true);
    rootFileCmd->SetParameter(filename);
    rootFileCmd->AvailableForStates(
            G4ApplicationState::G4State_PreInit,
            G4ApplicationState::G4State_Idle);
}

SimApplicationMessenger::~SimApplicationMessenger() {
}

void SimApplicationMessenger::SetNewValue(G4UIcommand* command, G4String newValues) {
    if (command == rootFileCmd) {
        RootEventWriter::getInstance()->setFileName(newValues);
    }
}

#include "SimApplication/RootPersistencyMessenger.h"

namespace sim {

RootPersistencyMessenger::RootPersistencyMessenger(RootPersistencyManager* rootIO)
    : rootIO(rootIO) {

    persistencyDir = new G4UIdirectory("/ldmx/persistency/");
    persistencyDir->SetGuidance("Persistency commands");

    rootDir = new G4UIdirectory("/ldmx/persistency/root/");
    rootDir->SetGuidance("ROOT persistency commands");

    rootFileCmd = new G4UIcommand("/ldmx/persistency/root/file", this);
    G4UIparameter* filename = new G4UIparameter("filename", 's', true);
    rootFileCmd->SetParameter(filename);
    rootFileCmd->AvailableForStates(
            G4ApplicationState::G4State_PreInit,
            G4ApplicationState::G4State_Idle);

}

RootPersistencyMessenger::~RootPersistencyMessenger() {
    delete rootFileCmd;
    delete rootDir;
    delete rootFileCmd;
}

void RootPersistencyMessenger::SetNewValue(G4UIcommand* command, G4String newValues) {
    if (command == rootFileCmd) {
        rootIO->setFileName(newValues);
    }
}

}

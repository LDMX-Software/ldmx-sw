#include "SimApplication/RootPersistencyMessenger.h"

namespace sim {

RootPersistencyMessenger::RootPersistencyMessenger(RootPersistencyManager* rootIO)
    : rootIO_(rootIO) {

    persistencyDir_ = new G4UIdirectory("/ldmx/persistency/");
    persistencyDir_->SetGuidance("Persistency commands");

    rootDir_ = new G4UIdirectory("/ldmx/persistency/root/");
    rootDir_->SetGuidance("ROOT persistency commands");

    rootFileCmd_ = new G4UIcommand("/ldmx/persistency/root/file", this);
    G4UIparameter* filename = new G4UIparameter("filename", 's', true);
    rootFileCmd_->SetParameter(filename);
    rootFileCmd_->AvailableForStates(
            G4ApplicationState::G4State_PreInit,
            G4ApplicationState::G4State_Idle);

}

RootPersistencyMessenger::~RootPersistencyMessenger() {
    delete rootFileCmd_;
    delete rootDir_;
    delete rootFileCmd_;
}

void RootPersistencyMessenger::SetNewValue(G4UIcommand* command, G4String newValues) {
    if (command == rootFileCmd_) {
        rootIO_->setFileName(newValues);
    }
}

}

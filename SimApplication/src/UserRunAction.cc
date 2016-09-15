#include "SimApplication/UserRunAction.h"

// LDMX
#include "Event/RootEventWriter.h"

UserRunAction::UserRunAction() {
}

UserRunAction::~UserRunAction() {
}

void UserRunAction::BeginOfRunAction(const G4Run* run) {

    std::cout << "UserRunAction::BeginOfRunAction" << run->GetRunID() << std::endl;

    RootEventWriter::getInstance()->open();
}

void UserRunAction::EndOfRunAction(const G4Run* run) {

    std::cout << "UserRunAction::EndOfRunAction - " << run->GetRunID() << std::endl;

    RootEventWriter::getInstance()->close();
}

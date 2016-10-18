#include "SimApplication/UserRunAction.h"

// LDMX
#include "Event/RootEventWriter.h"

using event::RootEventWriter;

namespace sim {

UserRunAction::UserRunAction() {
}

UserRunAction::~UserRunAction() {
}

void UserRunAction::BeginOfRunAction(const G4Run*) {
    RootEventWriter::getInstance()->open();
}

void UserRunAction::EndOfRunAction(const G4Run*) {
    RootEventWriter::getInstance()->close();
}

}

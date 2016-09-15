#include "SimApplication/UserEventAction.h"

// LDMX
#include "Event/RootEventWriter.h"

// STL
#include <iostream>

UserEventAction::UserEventAction() {
}

UserEventAction::~UserEventAction() {
}

void UserEventAction::BeginOfEventAction(const G4Event* event) {
    std::cout << "UserEventAction::BeginOfEventAction - " << event->GetEventID() << std::endl;
}

void UserEventAction::EndOfEventAction(const G4Event* event) {
    std::cout << "UserEventAction::EndOfEventAction - " << event->GetEventID() << std::endl;

    // Write out the ROOT event.
    RootEventWriter::getInstance()->write();
}

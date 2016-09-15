#include "SimApplication/UserEventAction.h"

// LDMX
#include "Event/RootEventWriter.h"

// Geant4
#include "G4RunManager.hh"
#include "G4Run.hh"

// STL
#include <iostream>
#include <stdio.h>
#include <time.h>

UserEventAction::UserEventAction() {
}

UserEventAction::~UserEventAction() {
}

void UserEventAction::BeginOfEventAction(const G4Event* event) {
    std::cout << "UserEventAction::BeginOfEventAction - " << event->GetEventID() << std::endl;

    EventHeader* header = RootEventWriter::getInstance()->getEvent()->header();
    header->setEventNumber(event->GetEventID());
    header->setTimestamp((int)time(NULL));
    header->setRun(G4RunManager::GetRunManager()->GetCurrentRun()->GetRunID());
}

void UserEventAction::EndOfEventAction(const G4Event* event) {
    std::cout << "UserEventAction::EndOfEventAction - " << event->GetEventID() << std::endl;

    // Write out the ROOT event.
    RootEventWriter::getInstance()->write();
}

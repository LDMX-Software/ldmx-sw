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

    // Set information on the current event header.
    EventHeader* header = RootEventWriter::getInstance()->getEvent()->getHeader();
    header->setEventNumber(event->GetEventID());
    header->setTimestamp((int)time(NULL));
    header->setRun(G4RunManager::GetRunManager()->GetCurrentRun()->GetRunID());
}

void UserEventAction::EndOfEventAction(const G4Event* event) {
    std::cout << "UserEventAction::EndOfEventAction - " << event->GetEventID() << std::endl;

    // Fill the current root event into the tree and then clear it.
    RootEventWriter::getInstance()->writeEvent();
}

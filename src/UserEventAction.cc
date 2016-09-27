#include "SimApplication/UserEventAction.h"

// LDMX
#include "Event/RootEventWriter.h"
#include "SimApplication/UserTrackInformation.h"

// Geant4
#include "G4RunManager.hh"
#include "G4Run.hh"

// STL
#include <iostream>
#include <stdio.h>
#include <time.h>

UserEventAction::UserEventAction()
    : simParticleBuilder(new SimParticleBuilder) {
}

UserEventAction::~UserEventAction() {
    delete simParticleBuilder;
}

void UserEventAction::BeginOfEventAction(const G4Event* event) {

    // Set information on the current event header.
    EventHeader* header = RootEventWriter::getInstance()->getEvent()->getHeader();
    header->setEventNumber(event->GetEventID());
    header->setTimestamp((int)time(NULL));
    header->setRun(G4RunManager::GetRunManager()->GetCurrentRun()->GetRunID());
}

void UserEventAction::EndOfEventAction(const G4Event* event) {

    // Build the SimParticle list for the output ROOT event.
    simParticleBuilder->buildSimParticles();

    // Assign SimParticle objects to SimTrackerHits.
    simParticleBuilder->assignTrackerHitSimParticles();

    // Assign SimParticle objects to SimCalorimeterHits.
    simParticleBuilder->assignCalorimeterHitSimParticles();

    // Fill the current ROOT event into the tree and then clear it.
    RootEventWriter::getInstance()->writeEvent();

    // Clear the registry of track information for processing the next event.
    TrackSummary::clearRegistry();

    std::cout << ">>> End Event " << event->GetEventID() << " <<<" << std::endl;
}

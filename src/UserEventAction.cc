#include "SimApplication/UserEventAction.h"

// LDMX
#include "Event/RootEventWriter.h"
#include "SimApplication/TrackMap.h"
#include "SimApplication/TrajectoryContainer.h"

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

void UserEventAction::BeginOfEventAction(const G4Event*) {

    // Install custom trajectory container for the event.
    G4EventManager::GetEventManager()->GetNonconstCurrentEvent()->SetTrajectoryContainer(new TrajectoryContainer);
}

void UserEventAction::EndOfEventAction(const G4Event* event) {

    // Build ROOT event header.
    EventHeader* header = RootEventWriter::getInstance()->getEvent()->getHeader();
    header->setEventNumber(event->GetEventID());
    header->setTimestamp((int)time(NULL));
    header->setRun(G4RunManager::GetRunManager()->GetCurrentRun()->GetRunID());

    // Build the SimParticle list for the output ROOT event.
    simParticleBuilder->buildSimParticles();

    // Assign SimParticle objects to SimTrackerHits.
    simParticleBuilder->assignTrackerHitSimParticles();

    // Assign SimParticle objects to SimCalorimeterHits.
    simParticleBuilder->assignCalorimeterHitSimParticles();

    // Fill the current ROOT event into the tree and then clear it.
    RootEventWriter::getInstance()->writeEvent();

    // Clear the global track map.
    TrackMap::getInstance()->clear();

    std::cout << ">>> End Event " << event->GetEventID() << " <<<" << std::endl;
}

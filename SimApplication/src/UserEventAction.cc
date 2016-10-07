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

    // Clear the current event object.
    RootEventWriter::getInstance()->getEvent()->Clear("");
}

void UserEventAction::EndOfEventAction(const G4Event* anEvent) {

    // Set ROOT event information.
    Event* rootEvent = RootEventWriter::getInstance()->getEvent();
    rootEvent->setEventNumber(anEvent->GetEventID());
    rootEvent->setTimestamp((int)time(NULL));
    rootEvent->setRun(G4RunManager::GetRunManager()->GetCurrentRun()->GetRunID());
    if (anEvent->GetPrimaryVertex(0)) {
        rootEvent->setWeight(anEvent->GetPrimaryVertex(0)->GetWeight());
        //std::cout << "set event weight: " << rootEvent->getWeight() << std::endl;
    }

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

    std::cout << ">>> End Event " << anEvent->GetEventID() << " <<<" << std::endl;
}

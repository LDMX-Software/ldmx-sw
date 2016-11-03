#include "SimApplication/RootPersistencyManager.h"

// LDMX
#include "Event/Event.h"
#include "SimApplication/G4CalorimeterHit.h"
#include "SimApplication/G4TrackerHit.h"

// Geant4
#include "G4RunManager.hh"

using event::Event;

namespace sim {

void RootPersistencyManager::writeHeader(const G4Event* anEvent, Event* outputEvent) {
    outputEvent->setEventNumber(anEvent->GetEventID());
    outputEvent->setTimestamp((int) time(NULL));
    outputEvent->setRun(G4RunManager::GetRunManager()->GetCurrentRun()->GetRunID());
    if (anEvent->GetPrimaryVertex(0)) {
        outputEvent->setWeight(anEvent->GetPrimaryVertex(0)->GetWeight());
    }
}

void RootPersistencyManager::writeHitsCollections(const G4Event* anEvent, Event* outputEvent) {
    G4HCofThisEvent* hce = anEvent->GetHCofThisEvent();
    if (hce == nullptr) {
        throw std::runtime_error("The HCE of this event is null!");
    }
    int nColl = hce->GetNumberOfCollections();
    for (int iColl = 0; iColl < nColl; iColl++) {
        G4VHitsCollection* hc = hce->GetHC(iColl);
        std::string collName = hc->GetName();
        int nHits = hc->GetSize();
        TClonesArray* outputColl = outputEvent->getCollection(hc->GetName());
        if (dynamic_cast<G4TrackerHitsCollection*>(hc) != nullptr) {
            for (int iHit = 0; iHit < nHits; iHit++) {
                G4TrackerHit* g4hit = (G4TrackerHit*) hc->GetHit(iHit);
                //std::cout << "creating new SimTrackerHit at " << outputColl->GetEntries() << std::endl;
                SimTrackerHit* simHit = (SimTrackerHit*) outputColl->ConstructedAt(outputColl->GetEntries());
                g4hit->setSimTrackerHit(simHit);
            }
        } else if (dynamic_cast<G4CalorimeterHitsCollection*>(hc) != nullptr) {
            for (int iHit = 0; iHit < nHits; iHit++) {
                G4CalorimeterHit* g4hit = (G4CalorimeterHit*) hc->GetHit(iHit);
                //std::cout << "creating new SimCalorimeterHit at " << outputColl->GetEntries() << std::endl;
                SimCalorimeterHit* simHit = (SimCalorimeterHit*) outputColl->ConstructedAt(outputColl->GetEntries());
                g4hit->setSimCalorimeterHit(simHit);
            }
        }
    }
}

G4bool RootPersistencyManager::Store(const G4Event* anEvent) {

    std::cout << "RootPersistencyManager::Store - saving event " << anEvent->GetEventID() << std::endl;

    // Get the current output event from the writer.
    Event* outputEvent = writer.getEvent();

    // Set basic event information.
    writeHeader(anEvent, outputEvent);

    // Copy hit objects from SD hit collections into the output event.
    writeHitsCollections(anEvent, outputEvent);

    // Set pointer to current G4Event.
    simParticleBuilder.setCurrentEvent(anEvent);

    // Build the SimParticle list for the output ROOT event.
    simParticleBuilder.buildSimParticles(outputEvent);

    // Assign SimParticle objects to SimTrackerHits.
    simParticleBuilder.assignTrackerHitSimParticles();

    // Assign SimParticle objects to SimCalorimeterHits.
    simParticleBuilder.assignCalorimeterHitSimParticles();

    // Fill the current ROOT event into the tree.
    writer.writeEvent();

    return true;
}

}

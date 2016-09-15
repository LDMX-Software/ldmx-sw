#include "SimApplication/TrackerSD.h"

// STL
#include <iostream>

// Geant4
#include "G4Step.hh"
#include "G4StepPoint.hh"
#include "G4SDManager.hh"

// LDMX
#include "Event/RootEventWriter.h"

TrackerSD::TrackerSD(G4String theName, G4String theCollectionName) :
    G4VSensitiveDetector(theName), hitsCollection(0), currentEvent(0) {

    // Add the collection name to vector of names.
    this->collectionName.push_back(theCollectionName);

    // Register this SD with the manager.
    G4SDManager::GetSDMpointer()->AddNewDetector(this);
}

TrackerSD::~TrackerSD() {
}

G4bool TrackerSD::ProcessHits(G4Step* aStep, G4TouchableHistory* ROhist) {

    // Create a new hit object using the ROOT event.
    SimTrackerHit* simTrackerHit =
            (SimTrackerHit*) currentEvent->addObject(collectionName[0]);
    G4TrackerHit* hit = new G4TrackerHit(simTrackerHit);

    // Set the edep.
    G4double edep = aStep->GetTotalEnergyDeposit();
    hit->getSimTrackerHit()->setEdep(edep);

    // Set the start position.
    G4StepPoint* prePoint = aStep->GetPreStepPoint();
    hit->setStartPosition(prePoint->GetPosition());

    // Set the end position.
    G4StepPoint* postPoint = aStep->GetPostStepPoint();
    hit->setEndPosition(postPoint->GetPosition());

    // Set the global time.
    hit->getSimTrackerHit()->setTime(aStep->GetTrack()->GetGlobalTime());

    /*
     * Compute and set the momentum.
     */
    double mag = (prePoint->GetMomentum().mag() + postPoint->GetMomentum().mag()) / 2;
    G4ThreeVector p = (postPoint->GetPosition() - prePoint->GetPosition());
    if (mag > 0) {
        p.setMag(mag);
    }
    hit->setMomentum(p);

    // TODO: set the ID from actual geometry info for system ID, layer, and sensor number
    hit->getSimTrackerHit()->setId(0);

    if (this->verboseLevel > 1) {
        std::cout << "Created new SimTrackerHit in detector " << this->GetName() << " ..." << std::endl;
        hit->Print();
        std::cout << std::endl;
    }

    hitsCollection->insert(hit);

    return true;
}

void TrackerSD::Initialize(G4HCofThisEvent* hce) {
    // Setup hits collection and the HC ID.
    hitsCollection = new G4TrackerHitsCollection(SensitiveDetectorName, collectionName[0]);
    G4int hcID = G4SDManager::GetSDMpointer()->GetCollectionID(collectionName[0]);
    hce->AddHitsCollection(hcID, hitsCollection);

    // Set ref to current ROOT output event.
    currentEvent = RootEventWriter::getInstance()->getEvent();
}

void TrackerSD::EndOfEvent(G4HCofThisEvent* hce) {
    /*
    G4int nHits = hitsCollection->entries();
    std::cout << "TrackerSD::EndOfEvent - " << GetName() << std::endl;
    std::cout << "There were " << nHits << " in the event." << std::endl;
    for (int i = 0; i < nHits; i++ ) {
        (*hitsCollection)[i]->Print();
    }
    */
}

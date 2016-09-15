#include "SimApplication/TrackerSD.h"

// STL
#include <iostream>

// Geant4
#include "G4Step.hh"
#include "G4StepPoint.hh"
#include "G4SDManager.hh"

// LDMX
#include "../include/SimApplication/G4TrackerHit.h"

TrackerSD::TrackerSD(G4String theName, G4String theCollectionName) :
    G4VSensitiveDetector(theName) {

    // Add the collection name to vector of names.
    this->collectionName.push_back(theCollectionName);

    // Register this SD with the manager.
    G4SDManager::GetSDMpointer()->AddNewDetector(this);
}

TrackerSD::~TrackerSD() {
}

G4bool TrackerSD::ProcessHits(G4Step* aStep, G4TouchableHistory* ROhist) {

    std::cout << "TrackerSD::ProcessHits - hello there" << std::endl;

    // Create a new sim tracker hit.
    G4TrackerHit* hit = new G4TrackerHit();

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

    // TODO: set the ID from actual geometry info
    hit->getSimTrackerHit()->setId(0);

    std::cout << "Created new SimTrackerHit in detector " << this->GetName() << " ...";
    hit->Print();
    std::cout << std::endl;

    return true;
}

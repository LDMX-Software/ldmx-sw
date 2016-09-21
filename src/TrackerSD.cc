#include "SimApplication/TrackerSD.h"

// STL
#include <iostream>

// Geant4
#include "G4Step.hh"
#include "G4StepPoint.hh"
#include "G4SDManager.hh"
#include "G4Geantino.hh"
#include "G4ChargedGeantino.hh"

// LDMX
#include "Event/RootEventWriter.h"
#include "DetDescr/IdField.h"

TrackerSD::TrackerSD(G4String name, G4String theCollectionName, int subdetId, DetectorId* detId) :
    G4VSensitiveDetector(name),
    hitsCollection(0),
    currentEvent(0),
    subdetId(subdetId),
    detId(detId) {

    // Add the collection name to vector of names.
    this->collectionName.push_back(theCollectionName);

    // Register this SD with the manager.
    G4SDManager::GetSDMpointer()->AddNewDetector(this);

    // Set the subdet ID as it will always be the same for every hit.
    detId->setFieldValue("subdet", subdetId);
}

TrackerSD::~TrackerSD() {
}

G4bool TrackerSD::ProcessHits(G4Step* aStep, G4TouchableHistory* ROhist) {

    // Determine if current particle of this step is a Geantino.
    G4ParticleDefinition* pdef = aStep->GetTrack()->GetDefinition();
    bool isGeantino = false;
    if (pdef == G4Geantino::Definition() || pdef == G4ChargedGeantino::Definition()) {
        isGeantino = true;
    }

    // Get the edep from the step.
    G4double edep = aStep->GetTotalEnergyDeposit();

    // Skip steps with no energy dep which come from non-Geantino particles.
    if (edep == 0.0 && !isGeantino) {
        if (verboseLevel > 1) {
            std::cout << "TrackerSD skipping step with zero edep" << std::endl << std::endl;
            return false;
        }
    }

    // Create a new hit object using the ROOT event.
    SimTrackerHit* simTrackerHit =
            (SimTrackerHit*) currentEvent->addObject(collectionName[0]);
    G4TrackerHit* hit = new G4TrackerHit(simTrackerHit);

    // Set the edep.
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

    /*
     * Set the 32-bit ID on the hit.
     */
    int layerNumber = prePoint->GetTouchableHandle()->GetHistory()->GetVolume(2)->GetCopyNo();
    detId->setFieldValue(1, layerNumber);
    hit->getSimTrackerHit()->setId(detId->pack());

    /*
     * Debug print.
     */
    if (this->verboseLevel > 0) {
        std::cout << "Created new SimTrackerHit in detector " << this->GetName()
                << " with subdet ID " << subdetId << " and layer " << layerNumber << " ..." << std::endl;
        hit->Print();
        std::cout << std::endl;
    }

    // Insert hit into current hits collection.
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
     * Debug print number of hits in this detector for the event.
     */
    if (this->verboseLevel > 0) {
        std::cout << GetName() << " had " << hitsCollection->entries()
                << " hits in event" << std::endl;
    }
    /*
    for (int i = 0; i < nHits; i++ ) {
        (*hitsCollection)[i]->Print();
    }
    */
}

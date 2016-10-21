#include "SimApplication/CalorimeterSD.h"

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

using event::RootEventWriter;

namespace sim {

CalorimeterSD::CalorimeterSD(G4String theName, G4String theCollectionName, int theSubdetId, DetectorID* theDetId) :
    G4VSensitiveDetector(theName),
    hitsCollection(0),
    currentEvent(0),
    subdetId(theSubdetId),
    detId(theDetId) {

    // Add the collection name to vector of names.
    this->collectionName.push_back(theCollectionName);

    // Register this SD with the manager.
    G4SDManager::GetSDMpointer()->AddNewDetector(this);

    // Set the subdet ID as it will always be the same for every hit.
    detId->setFieldValue("subdet", subdetId);
}

CalorimeterSD::~CalorimeterSD() {
}

G4bool CalorimeterSD::ProcessHits(G4Step* aStep, G4TouchableHistory*) {

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
            std::cout << "CalorimeterSD skipping step with zero edep" << std::endl << std::endl;
        }
        return false;
    }        

    // Create a new hit object using the ROOT event.
    //SimCalorimeterHit* simCalorimeterHit =
    //        (SimCalorimeterHit*) currentEvent->addObject(collectionName[0]);
    G4CalorimeterHit* hit = new G4CalorimeterHit(/*simCalorimeterHit*/);

    // Set the edep.
    hit->setEdep(edep);

    // Set the position.
    G4StepPoint* prePoint = aStep->GetPreStepPoint();
    G4StepPoint* postPoint = aStep->GetPostStepPoint();
    G4ThreeVector position = 0.5 * (prePoint->GetPosition() + postPoint->GetPosition());
    G4ThreeVector volumePosition = aStep->GetPreStepPoint()->GetTouchableHandle()->GetHistory()
            ->GetTopTransform().Inverse().TransformPoint(G4ThreeVector());
    hit->setPosition(position[0], position[1], volumePosition.z());

    // Set the global time.
    hit->setTime(aStep->GetTrack()->GetGlobalTime());

    // Set the ID on the hit.
    int layerNumber = prePoint->GetTouchableHandle()->GetHistory()->GetVolume(2)->GetCopyNo();
    detId->setFieldValue(1, layerNumber);
    hit->setID(detId->pack());

    // Set the track ID on the hit.
    hit->setTrackID(aStep->GetTrack()->GetTrackID());

    if (this->verboseLevel > 0) {
        std::cout << "Created new SimCalorimeterHit in detector " << this->GetName()
                << " with subdet ID " << subdetId << " and layer " << layerNumber << " ..." << std::endl;
        hit->Print();
        std::cout << std::endl;
    }

    hitsCollection->insert(hit);

    return true;
}

void CalorimeterSD::Initialize(G4HCofThisEvent* hce) {

    // Setup hits collection and the HC ID.
    hitsCollection = new G4CalorimeterHitsCollection(SensitiveDetectorName, collectionName[0]);
    G4int hcID = G4SDManager::GetSDMpointer()->GetCollectionID(collectionName[0]);
    hce->AddHitsCollection(hcID, hitsCollection);

    // Set ref to current ROOT output event.
    currentEvent = RootEventWriter::getInstance()->getEvent();
}

void CalorimeterSD::EndOfEvent(G4HCofThisEvent*) {
}

}

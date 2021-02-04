#include "SimCore/CalorimeterSD.h"

// STL
#include <iostream>

// Geant4
#include "G4ChargedGeantino.hh"
#include "G4Geantino.hh"
#include "G4SDManager.hh"
#include "G4Step.hh"
#include "G4StepPoint.hh"

namespace simcore {

CalorimeterSD::CalorimeterSD(G4String name, G4String theCollectionName)
    : G4VSensitiveDetector(name), hitsCollection_(0) {
  // Add the collection name to vector of names.
  this->collectionName.push_back(theCollectionName);

  // Register this SD with the manager.
  G4SDManager::GetSDMpointer()->AddNewDetector(this);
}

CalorimeterSD::~CalorimeterSD() {}

void CalorimeterSD::Initialize(G4HCofThisEvent* hce) {
  // Setup hits collection and the HC ID.
  hitsCollection_ =
      new G4CalorimeterHitsCollection(SensitiveDetectorName, collectionName[0]);
  G4int hcID = G4SDManager::GetSDMpointer()->GetCollectionID(collectionName[0]);
  hce->AddHitsCollection(hcID, hitsCollection_);
}

void CalorimeterSD::EndOfEvent(G4HCofThisEvent*) {
  // Print number of hits.
  if (this->verboseLevel > 0) {
    std::cout << GetName() << " had " << hitsCollection_->entries()
              << " hits in event" << std::endl;
  }

  // Print each hit in hits collection.
  if (this->verboseLevel > 1) {
    for (unsigned iHit = 0; iHit < hitsCollection_->GetSize(); iHit++) {
      (*hitsCollection_)[iHit]->Print();
    }
  }
}

}  // namespace simcore

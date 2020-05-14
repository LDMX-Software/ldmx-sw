#include "SimApplication/CalorimeterSD.h"

// STL
#include <iostream>

// Geant4
#include "G4Step.hh"
#include "G4StepPoint.hh"
#include "G4SDManager.hh"
#include "G4Geantino.hh"
#include "G4ChargedGeantino.hh"

namespace ldmx {

    CalorimeterSD::CalorimeterSD(G4String name, G4String theCollectionName, int subdetID) :
            G4VSensitiveDetector(name), hitsCollection_(0), subdet_(subdetID) {

        // Add the collection name to vector of names.
        this->collectionName.push_back(theCollectionName);

        // Register this SD with the manager.
        G4SDManager::GetSDMpointer()->AddNewDetector(this);

        // Set the subdet ID as it will always be the same for every hit.
        detID_->setFieldValue("subdet", subdet_);
    }

    CalorimeterSD::~CalorimeterSD() {
    }

    void CalorimeterSD::Initialize(G4HCofThisEvent* hce) {

        // Setup hits collection and the HC ID.
        hitsCollection_ = new G4CalorimeterHitsCollection(SensitiveDetectorName, collectionName[0]);
        G4int hcID = G4SDManager::GetSDMpointer()->GetCollectionID(collectionName[0]);
        hce->AddHitsCollection(hcID, hitsCollection_);
    }

    void CalorimeterSD::EndOfEvent(G4HCofThisEvent*) {
        // Print number of hits.
        if (this->verboseLevel > 0) {
            std::cout << GetName() << " had " << hitsCollection_->entries() << " hits in event" << std::endl;
        }

        // Print each hit in hits collection.
        if (this->verboseLevel > 1) {
            for (unsigned iHit = 0; iHit < hitsCollection_->GetSize(); iHit++) {
                (*hitsCollection_)[iHit]->Print();
            }
        }
    }

}

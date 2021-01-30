#include "SimCore/TrackerSD.h"

// STL
#include <iostream>

// Geant4
#include "G4ChargedGeantino.hh"
#include "G4Geantino.hh"
#include "G4SDManager.hh"
#include "G4Step.hh"
#include "G4StepPoint.hh"

// LDMX
#include "DetDescr/IDField.h"

namespace simcore {

TrackerSD::TrackerSD(G4String name, G4String theCollectionName, int subDetID)
    : G4VSensitiveDetector(name), hitsCollection_(0) {
  // Add the collection name to vector of names.
  this->collectionName.push_back(theCollectionName);

  // Register this SD with the manager.
  G4SDManager::GetSDMpointer()->AddNewDetector(this);

  // Set the subdet ID as it will always be the same for every hit.
  subDetID_ = ldmx::SubdetectorIDType(subDetID);
}

TrackerSD::~TrackerSD() {}

G4bool TrackerSD::ProcessHits(G4Step* aStep, G4TouchableHistory*) {
  // Determine if current particle of this step is a Geantino.
  G4ParticleDefinition* pdef = aStep->GetTrack()->GetDefinition();
  bool isGeantino = false;
  if (pdef == G4Geantino::Definition() ||
      pdef == G4ChargedGeantino::Definition()) {
    isGeantino = true;
  }

  // Get the edep from the step.
  G4double edep = aStep->GetTotalEnergyDeposit();

  // Skip steps with no energy dep which come from non-Geantino particles.
  if (edep == 0.0 && !isGeantino) {
    if (verboseLevel > 2) {
      std::cout << "TrackerSD skipping step with zero edep" << std::endl
                << std::endl;
    }
    return false;
  }

  // Create a new hit object.
  G4TrackerHit* hit = new G4TrackerHit();

  // Assign track ID for finding the SimParticle in post event processing.
  hit->setTrackID(aStep->GetTrack()->GetTrackID());

  // Set the edep.
  hit->setEdep(edep);

  // Set the start position.
  G4StepPoint* prePoint = aStep->GetPreStepPoint();
  // hit->setStartPosition(prePoint->GetPosition());

  // Set the end position.
  G4StepPoint* postPoint = aStep->GetPostStepPoint();
  // hit->setEndPosition(postPoint->GetPosition());

  G4ThreeVector start = prePoint->GetPosition();
  G4ThreeVector end = postPoint->GetPosition();

  // Set the mid position.
  G4ThreeVector mid = 0.5 * (start + end);
  hit->setPosition(mid.x(), mid.y(), mid.z());

  // Compute path length.
  G4double pathLength =
      sqrt(pow(start.x() - end.x(), 2) + pow(start.y() - end.y(), 2) +
           pow(start.z() - end.z(), 2));
  hit->setPathLength(pathLength);

  // Set the global time.
  hit->setTime(aStep->GetTrack()->GetGlobalTime());

  /*
   * Compute and set the momentum.
   */
  /*
   double mag = (prePoint->GetMomentum().mag() + postPoint->GetMomentum().mag())
   / 2; G4ThreeVector p = (postPoint->GetPosition() - prePoint->GetPosition());
   if (mag > 0) {
   p.setMag(mag);
   }
   */
  G4ThreeVector p = postPoint->GetMomentum();
  hit->setMomentum(p.x(), p.y(), p.z());

  /*
   * Set the 32-bit ID on the hit.
   */
  int copyNum =
      prePoint->GetTouchableHandle()->GetHistory()->GetVolume(2)->GetCopyNo();
  int layer = copyNum / 10;
  int module = copyNum % 10;
  ldmx::TrackerID id(subDetID_, layer, module);
  hit->setID(id.raw());
  hit->setLayerID(layer);
  hit->setModuleID(module);

  // Set energy and pdg code of SimParticle (common things requested)
  hit->setEnergy(postPoint->GetTotalEnergy());
  hit->setPdgID(aStep->GetTrack()->GetDynamicParticle()->GetPDGcode());

  /*
   * Debug print.
   */
  if (this->verboseLevel > 2) {
    hit->Print();
  }

  // Insert hit into current hits collection.
  hitsCollection_->insert(hit);

  return true;
}

void TrackerSD::Initialize(G4HCofThisEvent* hce) {
  // Setup hits collection and the HC ID.
  hitsCollection_ =
      new G4TrackerHitsCollection(SensitiveDetectorName, collectionName[0]);
  int hcID = G4SDManager::GetSDMpointer()->GetCollectionID(collectionName[0]);
  hce->AddHitsCollection(hcID, hitsCollection_);
}

void TrackerSD::EndOfEvent(G4HCofThisEvent*) {
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

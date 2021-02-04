
#include "SimCore/ScoringPlaneSD.h"
#include "DetDescr/SimSpecialID.h"

/*----------------*/
/*   C++ StdLib   */
/*----------------*/
#include <iostream>

/*~~~~~~~~~~~~*/
/*   Geant4   */
/*~~~~~~~~~~~~*/
#include "G4ChargedGeantino.hh"
#include "G4Geantino.hh"
#include "G4SDManager.hh"
#include "G4Step.hh"
#include "G4StepPoint.hh"

namespace simcore {

ScoringPlaneSD::ScoringPlaneSD(G4String name, G4String colName, int subDetID)
    : G4VSensitiveDetector(name) {
  // Add the collection name to vector of names.
  this->collectionName.push_back(colName);

  // Register this SD with the manager.
  G4SDManager::GetSDMpointer()->AddNewDetector(this);

  // at some point, confirm that the subDetID is as expected...
}

ScoringPlaneSD::~ScoringPlaneSD() {}

G4bool ScoringPlaneSD::ProcessHits(G4Step* step, G4TouchableHistory* history) {
  // Get the edep from the step.
  G4double edep = step->GetTotalEnergyDeposit();

  // Create a new hit object.
  G4TrackerHit* hit = new G4TrackerHit();

  // Assign track ID for finding the SimParticle in post event processing.
  hit->setTrackID(step->GetTrack()->GetTrackID());
  hit->setPdgID(step->GetTrack()->GetDynamicParticle()->GetPDGcode());

  // Set the edep.
  hit->setEdep(edep);

  // Set the start position.
  G4StepPoint* prePoint = step->GetPreStepPoint();

  // Set the end position.
  G4StepPoint* postPoint = step->GetPostStepPoint();

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
  hit->setTime(step->GetTrack()->GetGlobalTime());

  // Set the momentum
  G4ThreeVector p = postPoint->GetMomentum();
  hit->setMomentum(p.x(), p.y(), p.z());
  hit->setEnergy(postPoint->GetTotalEnergy());

  /*
   * Set the 32-bit ID on the hit.
   */
  int cpNumber = prePoint->GetTouchableHandle()->GetCopyNumber();
  ldmx::SimSpecialID id = ldmx::SimSpecialID::ScoringPlaneID(cpNumber);
  hit->setID(id.raw());

  /*
   * Debug print.
   */
  if (this->verboseLevel > 2) {
    hit->Print();
    std::cout << std::endl;
  }

  // Insert hit into current hits collection.
  hitsCollection_->insert(hit);

  return true;
}

void ScoringPlaneSD::Initialize(G4HCofThisEvent* hce) {
  // Setup hits collection and the HC ID.
  hitsCollection_ =
      new G4TrackerHitsCollection(SensitiveDetectorName, collectionName[0]);
  int hcID = G4SDManager::GetSDMpointer()->GetCollectionID(collectionName[0]);
  hce->AddHitsCollection(hcID, hitsCollection_);
}

void ScoringPlaneSD::EndOfEvent(G4HCofThisEvent*) {
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


#include "SimCore/SDs/ScoringPlaneSD.h"

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

ScoringPlaneSD::ScoringPlaneSD(const std::string& name,
                               simcore::ConditionsInterface& ci,
                               const framework::config::Parameters& params)
    : SensitiveDetector(name, ci, params) {
  collection_name_ = params.get<std::string>("collection_name");
  match_substr_ = params.get<std::string>("match_substr");
}

G4bool ScoringPlaneSD::ProcessHits(G4Step* step, G4TouchableHistory* history) {
  // Get the edep from the step.
  G4double edep = step->GetTotalEnergyDeposit();

  // Create a new hit object.
  ldmx::SimTrackerHit& hit{hits_.emplace_back()};

  // Assign track ID for finding the SimParticle in post event processing.
  hit.setTrackID(step->GetTrack()->GetTrackID());
  hit.setPdgID(step->GetTrack()->GetDynamicParticle()->GetPDGcode());

  // Set the edep.
  hit.setEdep(edep);

  // Set the start position.
  G4StepPoint* pre_point = step->GetPreStepPoint();

  // Set the end position.
  G4StepPoint* post_point = step->GetPostStepPoint();

  G4ThreeVector start = pre_point->GetPosition();
  G4ThreeVector end = post_point->GetPosition();

  // Set the mid position.
  G4ThreeVector mid = 0.5 * (start + end);
  hit.setPosition(mid.x(), mid.y(), mid.z());

  // Compute path length.
  G4double path_length =
      sqrt(pow(start.x() - end.x(), 2) + pow(start.y() - end.y(), 2) +
           pow(start.z() - end.z(), 2));
  hit.setPathLength(path_length);

  // Set the global time.
  hit.setTime(step->GetTrack()->GetGlobalTime());

  // Set the momentum
  G4ThreeVector p = post_point->GetMomentum();
  hit.setMomentum(p.x(), p.y(), p.z());
  hit.setEnergy(post_point->GetTotalEnergy());

  /*
   * Set the 32-bit ID on the hit.
   */
  int cp_number = pre_point->GetTouchableHandle()->GetCopyNumber();
  ldmx::SimSpecialID id = ldmx::SimSpecialID::scoringPlaneId(cp_number);
  hit.setID(id.raw());

  return true;
}

void ScoringPlaneSD::saveHits(framework::Event& event) {
  event.add(collection_name_, hits_);
}

}  // namespace simcore

DECLARE_SENSITIVEDETECTOR(simcore::ScoringPlaneSD)

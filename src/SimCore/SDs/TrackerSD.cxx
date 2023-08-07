#include "SimCore/SDs/TrackerSD.h"
// STL
#include <iostream>

// Geant4
#include "G4Step.hh"
#include "G4StepPoint.hh"

namespace simcore {

TrackerSD::TrackerSD(const std::string& name, simcore::ConditionsInterface& ci,
                     const framework::config::Parameters& p)
    : SensitiveDetector(name, ci, p) {
  subsystem_ = p.getParameter<std::string>("subsystem");
  collection_name_ = p.getParameter<std::string>("collection_name");

  subDetID_ = ldmx::SubdetectorIDType(p.getParameter<int>("subdet_id"));
}

G4bool TrackerSD::ProcessHits(G4Step* aStep, G4TouchableHistory*) {
  // Get the edep from the step.
  G4double edep = aStep->GetTotalEnergyDeposit();

  // Skip steps with no energy dep which come from non-Geantino particles.
  if (edep == 0.0 and not isGeantino(aStep)) {
    if (verboseLevel > 2) {
      std::cout << "TrackerSD skipping step with zero edep" << std::endl
                << std::endl;
    }
    return false;
  }

  // Create a new hit object.
  ldmx::SimTrackerHit& hit{hits_.emplace_back()};

  // Assign track ID for finding the SimParticle in post event processing.
  hit.setTrackID(aStep->GetTrack()->GetTrackID());

  // Set the edep.
  hit.setEdep(edep);

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
  hit.setPosition(mid.x(), mid.y(), mid.z());

  // Compute path length.
  G4double pathLength =
      sqrt(pow(start.x() - end.x(), 2) + pow(start.y() - end.y(), 2) +
           pow(start.z() - end.z(), 2));
  hit.setPathLength(pathLength);

  // Set the global time.
  hit.setTime(aStep->GetTrack()->GetGlobalTime());

  /*
   * Compute and set the momentum.
   */
  G4ThreeVector p = postPoint->GetMomentum();
  hit.setMomentum(p.x(), p.y(), p.z());

  /*
   * Set the 32-bit ID on the hit.
   */
  int copyNum =
      prePoint->GetTouchableHandle()->GetHistory()->GetVolume(2)->GetCopyNo();
  int layer = copyNum / 10;
  int module = copyNum % 10;
  ldmx::TrackerID id(subDetID_, layer, module);
  hit.setID(id.raw());
  hit.setLayerID(layer);
  hit.setModuleID(module);

  // Set energy and pdg code of SimParticle (common things requested)
  hit.setEnergy(postPoint->GetTotalEnergy());
  hit.setPdgID(aStep->GetTrack()->GetDynamicParticle()->GetPDGcode());

  return true;
}

}  // namespace simcore

DECLARE_SENSITIVEDETECTOR(simcore::TrackerSD)

#include "SimCore/SDs/TrigScintSD.h"

/*~~~~~~~~~~~~~~*/
/*   DetDescr   */
/*~~~~~~~~~~~~~~*/
#include "DetDescr/TrigScintID.h"

/*~~~~~~~~~~~~*/
/*   Geant4   */
/*~~~~~~~~~~~~*/
#include "G4Step.hh"
#include "G4StepPoint.hh"

namespace simcore {

TrigScintSD::TrigScintSD(const std::string& name,
                         simcore::ConditionsInterface& ci,
                         const framework::config::Parameters& p)
    : SensitiveDetector(name, ci, p) {
  module_id_ = p.get<int>("module_id");
  collection_name_ = p.get<std::string>("collection_name");
  vol_name_ = p.get<std::string>("volume_name");
  use_birks_law_ = p.get<bool>("use_birks_law");
  birks_const_one_ = p.get<double>("birks_const_one");
  birks_const_two_ = p.get<double>("birks_const_two");
}

G4bool TrigScintSD::ProcessHits(G4Step* step, G4TouchableHistory* history) {
  // Get the energy deposited by the particle during the step
  auto energy{step->GetTotalEnergyDeposit()};

  // If a non-Geantino particle doesn't deposit energy during the step,
  // skip processing it.
  if (energy == 0 and not isGeantino(step)) {
    ldmx_log(trace) << "No energy deposited in step, skipping.";
    return false;
  }

  // Create a new instance of a calorimeter hit
  //  emplace_back returns a *reference* to the hit that was constructed
  //  and we should keep that reference so that we are editing the correct hit
  ldmx::SimCalorimeterHit& hit = hits_.emplace_back();

  G4StepPoint* prePoint = step->GetPreStepPoint();
  G4StepPoint* postPoint = step->GetPostStepPoint();
  if (prePoint == nullptr || postPoint == nullptr) {
    return false;
  }

  // A Geant4 "touchable" is a way to uniquely identify a particular volume,
  // short for touchable detector element. See the detector definition and
  // response section of the Geant4 application developers manual for details.
  //
  // The TouchableHandle is just a reference counted pointer to a
  // G4TouchableHistory object, which is a concrete implementation of a
  // G4Touchable interface.
  if (!prePoint->GetTouchableHandle()) {
    return false;
  }
  auto touchableHistory{prePoint->GetTouchableHandle()->GetHistory()};
  // Affine transform for converting between local and global coordinates
  auto topTransform{touchableHistory->GetTopTransform()};
  // Set the hit position
  auto position{0.5 * (prePoint->GetPosition() + postPoint->GetPosition())};

  // Convert the center of the bar to its corresponding global position
  auto volumePosition{topTransform.Inverse().TransformPoint(G4ThreeVector())};
  hit.setPosition(position[0], position[1], volumePosition.z());

  // Get the track associated with this step
  auto track{step->GetTrack()};

  // Set the ID on the hit.
  auto bar{track->GetVolume()->GetCopyNo()};
  ldmx::TrigScintID id(module_id_, bar);
  hit.setID(id.raw());

  // Implement Birks law
  G4double birks_factor(1.0);
  G4double step_length = step->GetStepLength() / CLHEP::cm;
  // Do not apply Birks for gamma deposits!
  // Check, cut if necessary.
  if (step_length > 1.0e-6) {
    G4double rho = step->GetPreStepPoint()->GetMaterial()->GetDensity() /
                   (CLHEP::g / CLHEP::cm3);
    G4double dedx = energy / (rho * step_length);  //[MeV*cm^2/g]
    birks_factor =
        1.0 / (1.0 + birks_const_one_ * dedx + birks_const_two_ * dedx * dedx);
    // Birks law is only applied to charged particles
    if (step->GetTrack()->GetDefinition()->GetPDGCharge() == 0) {
      birks_factor = 1.0;
    }
  }

  // update the energy to include birks_factor
  if (use_birks_law_) {
    ldmx_log(trace) << "Applying Birks law with factor: " << birks_factor;
    energy *= birks_factor;
  }

  // add single contrib to this calorimeter hit
  //  IncidentID - this track's ID
  //  Track ID
  //  PDG ID
  //  energy deposited
  //  global time of this hit
  hit.addContrib(track->GetTrackID(), track->GetTrackID(),
                 track->GetParticleDefinition()->GetPDGEncoding(), energy,
                 track->GetGlobalTime());

  // Step details
  hit.setPathLength(step->GetStepLength());
  hit.setVelocity(track->GetVelocity());
  // Convert pre/post step position from global coordinates to coordinates
  // within the scintillator bar
  const auto localPreStepPoint{
      topTransform.TransformPoint(prePoint->GetPosition())};
  const auto localPostStepPoint{
      topTransform.TransformPoint(postPoint->GetPosition())};
  hit.setPreStepPosition(localPreStepPoint[0], localPreStepPoint[1],
                         localPreStepPoint[2]);

  hit.setPostStepPosition(localPostStepPoint[0], localPostStepPoint[1],
                          localPostStepPoint[2]);

  hit.setPreStepTime(prePoint->GetGlobalTime());
  hit.setPostStepTime(postPoint->GetGlobalTime());

  return true;
}

}  // namespace simcore

DECLARE_SENSITIVEDETECTOR(simcore::TrigScintSD)

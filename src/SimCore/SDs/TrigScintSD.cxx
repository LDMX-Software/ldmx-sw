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
      module_id_ = p.getParameter<int>("module_id");
      collection_name_ = p.getParameter<std::string>("collection_name");
      vol_name_ = p.getParameter<std::string>("volume_name");
    }

TrigScintSD::~TrigScintSD() {}

G4bool TrigScintSD::ProcessHits(G4Step* step, G4TouchableHistory* history) {
  // Get the energy deposited by the particle during the step
  auto energy{step->GetTotalEnergyDeposit()};

  // If a non-Geantino particle doesn't deposit energy during the step,
  // skip processing it.
  if (energy == 0 and not isGeantino(step)) return false;

  // Create a new instance of a calorimeter hit
  //  emplace_back returns a *reference* to the hit that was constructed
  //  and we should keep that reference so that we are editing the correct hit
  ldmx::SimCalorimeterHit& hit = hits_.emplace_back();

  G4StepPoint* prePoint = step->GetPreStepPoint();
  G4StepPoint* postPoint = step->GetPostStepPoint();
  // Set the hit position
  auto position{0.5 * (step->GetPreStepPoint()->GetPosition() +
                       step->GetPostStepPoint()->GetPosition())};
  auto volumePosition{step->GetPreStepPoint()
                          ->GetTouchableHandle()
                          ->GetHistory()
                          ->GetTopTransform()
                          .Inverse()
                          .TransformPoint(G4ThreeVector())};
  auto position{0.5 * (prePoint->GetPosition() +
                       postPoint->GetPosition())};
  hit.setPosition(position[0], position[1], volumePosition.z());

  // Get the track associated with this step
  auto track{step->GetTrack()};

  // Set the ID on the hit.
  auto bar{track->GetVolume()->GetCopyNo()};
  ldmx::TrigScintID id(module_id_, bar);
  hit.setID(id.raw());

  // add single contrib to this calorimeter hit
  //  IncidentID - this track's ID
  //  Track ID
  //  PDG ID
  //  energy deposited
  //  global time of this hit
  hit.addContrib(track->GetTrackID(), track->GetTrackID(),
                 track->GetParticleDefinition()->GetPDGEncoding(), energy,
                 track->GetGlobalTime());

  return true;
}

}  // namespace simcore

DECLARE_SENSITIVEDETECTOR(simcore::TrigScintSD)

#include "SimCore/SDs/EcalSD.h"

// Geant4
#include "G4Polyhedron.hh"
#include "G4Step.hh"
#include "G4StepPoint.hh"
#include "G4VSolid.hh"

/*~~~~~~~~~~~~~~*/
/*   DetDescr   */
/*~~~~~~~~~~~~~~*/
#include "DetDescr/EcalGeometry.h"
#include "DetDescr/EcalID.h"

namespace simcore {

const std::string EcalSD::COLLECTION_NAME = "EcalSimHits";

EcalSD::EcalSD(const std::string& name, simcore::ConditionsInterface& ci,
               const framework::config::Parameters& p)
    : SensitiveDetector(name, ci, p) {
  enableHitContribs_ = p.getParameter<bool>("enableHitContribs");
  compressHitContribs_ = p.getParameter<bool>("compressHitContribs");
}

G4bool EcalSD::ProcessHits(G4Step* aStep, G4TouchableHistory*) {
  static const int layer_depth = 2;  // index depends on GDML implementation
  const auto& geometry = getCondition<ldmx::EcalGeometry>(
      ldmx::EcalGeometry::CONDITIONS_OBJECT_NAME);

  // Get the edep from the step.
  G4double edep = aStep->GetTotalEnergyDeposit();

  // Skip steps with no energy dep which come from non-Geantino particles.
  if (edep == 0.0 and not isGeantino(aStep)) {
    if (verboseLevel > 2) {
      G4cout << "CalorimeterSD skipping step with zero edep." << G4endl
             << G4endl;
    }
    return false;
  }

  // Compute the hit position
  G4StepPoint* prePoint = aStep->GetPreStepPoint();
  G4StepPoint* postPoint = aStep->GetPostStepPoint();
  G4ThreeVector position =
      0.5 * (prePoint->GetPosition() + postPoint->GetPosition());

  // Create the ID for the hit.
  int cpynum = aStep->GetPreStepPoint()
                   ->GetTouchableHandle()
                   ->GetHistory()
                   ->GetVolume(layer_depth)
                   ->GetCopyNo();
  int layerNumber;
  layerNumber = int(cpynum / 7);
  int module_position = cpynum % 7;
  /**
   * DEBUG
   *  this printout is helpful when developing the GDML and/or EcalGeometry
   *  since Geant4 will probe where _exactly_ the GDML sensitive volumes are
  std::cout
    << "(" << position[0] << ", " << position[1] << ", " << position[2] << ") "
    << cpynum << " -> layer " << layerNumber
    << " module " << module_position
    << std::endl;
   */

  // fastest, but need to trust module number between GDML and EcalGeometry
  // match
  ldmx::EcalID id =
      geometry.getID(position[0], position[1], layerNumber, module_position);

  // medium, only need to trust z-layer positions in GDML and EcalGeometry match
  //    helpful for debugging any issues where transverse position is not
  //    matching between the GDML and EcalGeometry
  // ldmx::EcalID id = geometry.getID(position[0], position[1], layerNumber);

  // slowest, completely rely on EcalGeometry
  //    this is helpful for validating the EcalGeometry implementation and
  //    configuration since this will be called with any hit position that
  //    is inside of the configured SD volumes from Geant4's point of view
  // ldmx::EcalID id = geometry.getID(position[0], position[1], position[2]);

  if (hits_.find(id) == hits_.end()) {
    // hit in empty cell
    auto& hit = hits_[id];
    hit.setID(id.raw());
    /**
     * convert position to center of cell position
     *
     * This is the behavior that has been done in the past,
     * although it is completely redundant with the ID information
     * already deduced. It would probably help us more if we
     * persisted the actual simulated position of the hit rather
     * than the cell center; however, that is up for more discussion.
     */
    auto [x, y, z] = geometry.getPosition(id);
    hit.setPosition(x, y, z);
  }

  auto& hit = hits_[id];

  // hit variables
  auto track = aStep->GetTrack();
  auto time = track->GetGlobalTime();
  auto track_id = track->GetTrackID();
  auto pdg = track->GetParticleDefinition()->GetPDGEncoding();

  if (enableHitContribs_) {
    int contrib_i = hit.findContribIndex(track_id, pdg);
    if (compressHitContribs_ and contrib_i != -1) {
      hit.updateContrib(contrib_i, edep, time);
    } else {
      hit.addContrib(getTrackMap().findIncident(track_id), track_id, pdg, edep,
                     time);
    }
  } else {
    // no hit contribs and hit already exists
    hit.setEdep(hit.getEdep() + edep);
    if (time < hit.getTime() or hit.getTime() == 0) {
      hit.setTime(time);
    }
  }

  return true;
}

void EcalSD::saveHits(framework::Event& event) {
  // squash hits into list
  std::vector<ldmx::SimCalorimeterHit> hits;
  hits.reserve(hits_.size());
  for (const auto& [id, hit] : hits_) hits.push_back(hit);
  event.add(COLLECTION_NAME, hits);
}

}  // namespace simcore

DECLARE_SENSITIVEDETECTOR(simcore::EcalSD)

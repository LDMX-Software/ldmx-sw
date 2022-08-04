#include "SimCore/SDs/EcalSD.h"

// Geant4
#include "G4Polyhedron.hh"
#include "G4Step.hh"
#include "G4StepPoint.hh"
#include "G4VSolid.hh"

/*~~~~~~~~~~~~~~*/
/*   DetDescr   */
/*~~~~~~~~~~~~~~*/
#include "DetDescr/EcalID.h"

namespace simcore {

const std::string EcalSD::COLLECTION_NAME = "EcalSimHits";

EcalSD::EcalSD(const std::string& name, simcore::ConditionsInterface& ci,
               const framework::config::Parameters& p)
    : SensitiveDetector(name, ci, p) {
      enableHitContribs_ = p.getParameter<bool>("enableHitContribs");
      compressHitContribs_ = p.getParameter<bool>("compressHitContribs");
    }

EcalSD::~EcalSD() {}

G4bool EcalSD::ProcessHits(G4Step* aStep, G4TouchableHistory*) {
  static const int layer_depth = 2; // index depends on GDML implementation
  const auto& hitMap = getCondition<ldmx::EcalHexReadout>(
      ldmx::EcalHexReadout::CONDITIONS_OBJECT_NAME);

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

  ldmx::EcalID partialId =
      hitMap.getCellModuleID(position[0], position[1]);
  ldmx::EcalID id(layerNumber, module_position, partialId.cell());

  if (hits_.find(id) == hits_.end()) {
    // hit in empty cell
    auto& hit = hits_[id];
    hit.setID(id.raw());
    double x, y, z;
    hitMap.getCellAbsolutePosition(id, x, y, z);
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
      hit.addContrib(getTrackMap().findIncident(track_id), track_id,
                     pdg, edep, time);
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
  for (const auto& [ id, hit] : hits_) hits.push_back(hit);
  event.add(COLLECTION_NAME, hits);
  // clear last events hits
  hits_.clear();
}

}  // namespace simcore

DECLARE_SENSITIVEDETECTOR(simcore::EcalSD)

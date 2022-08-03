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
    : SensitiveDetector(name, ci, p) {}

EcalSD::~EcalSD() {}

G4bool EcalSD::ProcessHits(G4Step* aStep, G4TouchableHistory*) {
  const ldmx::EcalHexReadout& hitMap = getCondition<ldmx::EcalHexReadout>(
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

  // Create a new cal hit at back of list
  ldmx::SimCalorimeterHit& hit{hits_.emplace_back()};

  // Compute the hit position using the utility function.
  G4ThreeVector hitPosition = getHitPosition(aStep);
  hit.setPosition(hitPosition.x(), hitPosition.y(), hitPosition.z());

  // Create the ID for the hit.
  int cpynum = aStep->GetPreStepPoint()
                   ->GetTouchableHandle()
                   ->GetHistory()
                   ->GetVolume(2)  // this index depends on GDML implementation
                   ->GetCopyNo();
  int layerNumber;
  layerNumber = int(cpynum / 7);
  int module_position = cpynum % 7;

  ldmx::EcalID partialId =
      hitMap.getCellModuleID(hitPosition[0], hitPosition[1]);
  ldmx::EcalID id(layerNumber, module_position, partialId.cell());
  hit.setID(id.raw());

  // add one contributor for this hit with
  //  ID of ancestor incident on Cal-Region
  //  ID of this track
  //  PDG of this track
  //  EDEP 
  //  time of this hit
  const G4Track* track = aStep->GetTrack();
  int track_id = track->GetTrackID();
  hit.addContrib(getTrackMap().findIncident(track_id), track_id,
                 track->GetParticleDefinition()->GetPDGEncoding(),
                 edep, track->GetGlobalTime());

  if (this->verboseLevel > 2) {
    G4cout << "Created new SimCalorimeterHit in detector " << this->GetName()
           << " with subdet ID " << id << " ...";
    hit.Print();
    G4cout << G4endl;
  }

  return true;
}

G4ThreeVector EcalSD::getHitPosition(G4Step* aStep) {
  /**
   * Set initial hit position from midpoint of the step.
   */
  G4StepPoint* prePoint = aStep->GetPreStepPoint();
  G4StepPoint* postPoint = aStep->GetPostStepPoint();
  G4ThreeVector position =
      0.5 * (prePoint->GetPosition() + postPoint->GetPosition());

  /*
   * Get the volume position in global coordinates, which for the ECal is the
   * center of the front face of the sensor.
   */
  G4ThreeVector volumePosition = aStep->GetPreStepPoint()
                                     ->GetTouchableHandle()
                                     ->GetHistory()
                                     ->GetTopTransform()
                                     .Inverse()
                                     .TransformPoint(G4ThreeVector());

  // Get the solid from this step.
  G4VSolid* solid = prePoint->GetTouchableHandle()
                        ->GetVolume()
                        ->GetLogicalVolume()
                        ->GetSolid();
  auto it = polyMap_.find(solid);
  G4Polyhedron* poly;
  if (it == polyMap_.end()) {
    poly = solid->CreatePolyhedron();
    polyMap_[solid] = poly;
  } else {
    poly = polyMap_[solid];
  }

  /**
   * Use facet info of the solid for setting Z to the sensor midpoint.
   */
  G4Point3D iNodes[4];
  G4int n;
  poly->GetFacet(1, n, iNodes);
  G4double zstart = iNodes[1][2];
  G4double zend = iNodes[0][2];
  G4double zmid = (zstart - zend) / 2;
  position.setZ(volumePosition.z() + zmid);

  return position;
}

}  // namespace simcore

DECLARE_SENSITIVEDETECTOR(simcore, EcalSD)

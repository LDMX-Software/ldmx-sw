#include "SimCore/SDs/EcalSD.h"

namespace simcore {

const std::string EcalSD::COLLECTION_NAME = "EcalSimHits";

EcalSD::EcalSD(const std::string& name, simcore::ConditionsInterface& ci,
               const framework::config::Parameters& p)
    : SensitiveDetector(name, ci, p) {
  enable_hit_contribs_ = p.get<bool>("enable_hit_contribs");
  compress_hit_contribs_ = p.get<bool>("compress_hit_contribs");
  max_origin_track_id_ = p.get<int>("max_origin_track_id");
}

G4bool EcalSD::ProcessHits(G4Step* aStep, G4TouchableHistory*) {
  static const int layer_depth = 2;  // index depends on GDML implementation
  const auto& geometry = getCondition<ldmx::EcalGeometry>(
      ldmx::EcalGeometry::CONDITIONS_OBJECT_NAME);

  // Get the edep from the step.
  G4double edep = aStep->GetTotalEnergyDeposit();

  // Skip steps with no energy dep which come from non-Geantino particles.
  if (edep == 0.0 and not isGeantino(aStep)) {
    ldmx_log(trace) << "CalorimeterSD skipping step with zero edep.";
    return false;
  }

  // Compute the hit position
  G4StepPoint* pre_point = aStep->GetPreStepPoint();
  G4StepPoint* post_point = aStep->GetPostStepPoint();
  G4ThreeVector position =
      0.5 * (pre_point->GetPosition() + post_point->GetPosition());

  // Create the ID for the hit.
  int cpynum{0};  // Initialize cpynum to 0

  auto pre_step_point = aStep->GetPreStepPoint();
  if (pre_step_point) {
    const auto& touchable_handle = pre_step_point->GetTouchableHandle();
    if (touchable_handle) {
      auto history = touchable_handle->GetHistory();
      if (history) {
        auto volume = history->GetVolume(layer_depth);
        if (volume) {
          cpynum = volume->GetCopyNo();
        }
      }
    }
  }
  int layer_number;
  layer_number = cpynum / 7;
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
      geometry.getID(position.x(), position.y(), layer_number, module_position);

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
    hit.setPosition(position.x(), position.y(), position.z());
  }

  auto& hit = hits_[id];

  // hit variables
  auto track = aStep->GetTrack();
  auto time = track->GetGlobalTime();
  auto track_id = track->GetTrackID();
  auto pdg = track->GetParticleDefinition()->GetPDGEncoding();

  if (enable_hit_contribs_) {
    int contrib_i = hit.findContribIndex(track_id, pdg);
    if (compress_hit_contribs_ and contrib_i != -1) {
      hit.updateContrib(contrib_i, edep, time);
    } else {
      auto map{getTrackMap()};
      auto incident{map.findIncident(track_id)};
      // default "origin" is just the same as incident
      // "origin" checks if a hit "originates" from one of the earliest
      // track IDs (i.e. probably one of the primaries)
      int origin{incident};
      for (int i{1}; i < max_origin_track_id_; ++i) { // här ska vi inte leta efter <6 track id utan vi ska kolla på event_indx
        if (map.isDescendant(track_id, i, 100)) {
          origin = i;
          break;
        }
      }
      hit.addContrib(incident, track_id, pdg, edep, time, origin);
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

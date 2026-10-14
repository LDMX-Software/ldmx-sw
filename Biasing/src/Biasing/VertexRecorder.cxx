/*~~~~~~~~~~~~~*/
/*   Biasing   */
/*~~~~~~~~~~~~~*/
#include "Biasing/VertexRecorder.h"

/*~~~~~~~~~~~~~*/
/*   Geant4    */
/*~~~~~~~~~~~~~*/
#include "G4VPhysicalVolume.hh"

/*~~~~~~~~~~~~~*/
/*   SimCore   */
/*~~~~~~~~~~~~~*/
#include "SimCore/Event/VertexInfo.h"
#include "SimCore/G4User/UserTrackInformation.h"
#include "SimCore/VertexRecordStore.h"

namespace biasing {

VertexRecorder::VertexRecorder(const std::string &name,
                               framework::config::Parameters &parameters)
    : simcore::UserAction(name, parameters) {
  region_ = parameters.get<std::string>("region", "");
  process_ = parameters.get<std::string>("process", "");
}

void VertexRecorder::beginOfEventAction(const G4Event *) {
  // Remove all old stuff
  simcore::VertexRecordStore::getInstance().clear();
  product_track_ids_.clear();
}

void VertexRecorder::stepping(const G4Step *step) {
  auto track{step->GetTrack()};

  // Only primaries (works for signal + pileup primaries alike).
  if (track->GetParentID() != 0)
    return;

  // Was this step an interaction of the configured process?
  // An empty configured process name means "record any process".
  auto proc{step->GetPostStepPoint()->GetProcessDefinedStep()};
  if (!proc || (!process_.empty() && proc->GetProcessName() != process_))
    return;

  // The vertex is at the post-step point; remember the volume
  // containing it so both the region cut and the record use the same one.
  const G4VPhysicalVolume *vertexVolume{
      step->GetPostStepPoint()->GetPhysicalVolume()};

  // Region cut, e.g. only the target.
  if (!region_.empty()) {
    const G4LogicalVolume *vertexLogical{
        vertexVolume ? vertexVolume->GetLogicalVolume() : nullptr};
    const G4Region *vertexRegion{vertexLogical ? vertexLogical->GetRegion()
                                               : nullptr};
    if (!vertexRegion || vertexRegion->GetName() != region_)
      return;
  }

  // Pre-step point = the incoming particle's kinematics entering the vertex.
  const G4StepPoint *pre{step->GetPreStepPoint()};
  ldmx::VertexRecord::ParticleInfo incoming;
  incoming.track_id_ = track->GetTrackID();
  incoming.pdg_id_ = track->GetParticleDefinition()->GetPDGEncoding();
  incoming.energy_ =
      pre->GetKineticEnergy() + track->GetParticleDefinition()->GetPDGMass();
  const G4ThreeVector &p{pre->GetMomentum()}, &x{pre->GetPosition()};
  incoming.px_ = p.x();
  incoming.py_ = p.y();
  incoming.pz_ = p.z();
  incoming.x_ = x.x();
  incoming.y_ = x.y();
  incoming.z_ = x.z();
  incoming.time_ = pre->GetGlobalTime();

  ldmx::VertexRecord vertex;
  vertex.setIncomingParticle(incoming);
  vertex.setProcessName(proc->GetProcessName());
  vertex.setVolumeName(vertexVolume ? vertexVolume->GetName()
                                    : track->GetVolume()->GetName());

  // All secondaries of this step are the vertex products; tag them so they
  // are saved and their track IDs link them back to this vertex.
  const auto *secondaries = step->GetSecondaryInCurrentStep();
  if (secondaries) {
    for (const auto *secondary : *secondaries) {
      if (secondary->GetCreatorProcess() != proc)
        continue;
      simcore::UserTrackInformation::get(secondary)->setSaveFlag(true);
      product_track_ids_.push_back(secondary->GetTrackID());

      // Record the product's initial state alongside the incoming particle.
      ldmx::VertexRecord::ParticleInfo product;
      product.track_id_ = secondary->GetTrackID();
      product.pdg_id_ = secondary->GetParticleDefinition()->GetPDGEncoding();
      product.energy_ = secondary->GetKineticEnergy() +
                        secondary->GetParticleDefinition()->GetPDGMass();
      const G4ThreeVector &pp{secondary->GetMomentum()};
      const G4ThreeVector &xp{secondary->GetPosition()};
      product.px_ = pp.x();
      product.py_ = pp.y();
      product.pz_ = pp.z();
      product.x_ = xp.x();
      product.y_ = xp.y();
      product.z_ = xp.z();
      product.time_ = secondary->GetGlobalTime();
      vertex.addOutgoingParticle(product);
    }
  }

  simcore::VertexRecordStore::getInstance().addVertex(vertex);

  ldmx_log(trace) << "recorded vertex: p_in = (" << p.x() << ", " << p.y()
                  << ", " << p.z() << ") MeV, "
                  << vertex.getNumOutgoingParticles() << " products";
}
} // namespace biasing

DECLARE_ACTION(biasing::VertexRecorder)

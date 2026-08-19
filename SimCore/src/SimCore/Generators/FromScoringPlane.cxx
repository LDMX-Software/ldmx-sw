#include "SimCore/Generators/FromScoringPlane.h"

#include "SimCore/Event/SimTrackerHit.h"
#include "DetDescr/SimSpecialID.h"
#include <unordered_map>
#include "G4Event.hh"
#include "G4PrimaryParticle.hh"

namespace simcore {
namespace generators {

FromScoringPlane::FromScoringPlane(
    const std::string& name, const framework::config::Parameters& parameters)
    : PrimaryGenerator(name, parameters),
      coll_name_{parameters.get<std::string>("coll_name")},
      pass_name_{parameters.get<std::string>("pass_name")},
      select_planes_{parameters.get<std::vector<int>>("select_planes")}
      {}

void FromScoringPlane::PrepEvent(const framework::Event& event) {
  ldmx_log(info) << "preparing event";
  const auto& scoring_plane_hits{event.getCollection<ldmx::SimTrackerHit>(coll_name_, pass_name_)};
  std::unordered_map<int, std::vector<const ldmx::SimTrackerHit*>> hits_by_track_id;
  for (const auto& hit : scoring_plane_hits) {
    ldmx::SimSpecialID id(hit.getID());
    bool keep{
      select_planes_.empty() or
      std::find(select_planes_.begin(), select_planes_.end(), id.plane()) != select_planes_.end()
    };
    // do filtering by layer here
    ldmx_log(debug) << "hit with plane = " << id.plane()
                    << " is " << (keep ? "used" : "ignored");
    if (keep) {
      // sort hits that we are keeping by track ID
      hits_by_track_id[hit.getTrackID()].push_back(&hit);
    }
  }

  // copy earliest hit by track Id in as primary
  primary_vertices_.clear();
  for (auto& [track_id, hits] : hits_by_track_id) {
    auto earliest_hit_it = std::min_element(
        hits.begin(), hits.end(),
        [](const auto& hit_lhs, const auto& hit_rhs) {
          return hit_lhs->getTime() < hit_rhs->getTime();
    });
    const auto* earliest_hit = (*earliest_hit_it);

    G4PrimaryParticle* particle = new G4PrimaryParticle;
    // proper PDG also copies in mass and charge
    particle->SetPDGcode(earliest_hit->getPdgID());

    auto momentum{earliest_hit->getMomentum()};
    auto energy{earliest_hit->getEnergy()};
    particle->Set4Momentum(momentum[0], momentum[1], momentum[2], energy);
    // probably not correct from a relativistic perspective
    particle->SetProperTime(earliest_hit->getTime());
    // label this particle as a primary for purposes of serialization
    UserPrimaryParticleInformation* uppi = new UserPrimaryParticleInformation;
    uppi->setHepEvtStatus(1);
    particle->SetUserInformation(uppi);
    // can I copy in the old track_id?
    // particle->SetTrackId(track_id);

    auto pos{earliest_hit->getPosition()};
    G4PrimaryVertex* vertex = new G4PrimaryVertex;
    vertex->SetPosition(pos[0], pos[1], pos[2]);
    // not setting a weight
    vertex->SetPrimary(particle);

    primary_vertices_.push_back(vertex);
  }
}

void FromScoringPlane::GeneratePrimaryVertex(G4Event* anEvent) {
  ldmx_log(info) << "generating primary vertex";
  for (auto* primary_vertex : primary_vertices_) {
    anEvent->AddPrimaryVertex(primary_vertex);
  }
}

void FromScoringPlane::RecordConfig(const std::string& id,
                                       ldmx::RunHeader& rh) {
  rh.setStringParameter(id + " Class",
                        "simcore::generators::FromScoringPlane");
  rh.setStringParameter(id + " Coll Name", coll_name_);
  rh.setStringParameter(id + " Pass Name", pass_name_);
}

}  // namespace generators
}  // namespace simcore

DECLARE_GENERATOR(simcore::generators::FromScoringPlane)

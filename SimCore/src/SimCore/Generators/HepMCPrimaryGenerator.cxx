#include "SimCore/Generators/HepMCPrimaryGenerator.h"

#include <algorithm>

#include "G4RunManager.hh"
#include "SimCore/G4User/UserPrimaryParticleInformation.h"

namespace simcore {
namespace generators {

HepMCPrimaryGenerator::HepMCPrimaryGenerator(
    const std::string& name, const framework::config::Parameters& parameters)
    : PrimaryGenerator(name, parameters),
      file_path_{parameters.get<std::string>("file_path")},
      reader_{file_path_},
      vertex_{parameters.get<std::vector<double>>("vertex")} {}

void HepMCPrimaryGenerator::GeneratePrimaryVertex(G4Event* anEvent) {
  std::unique_ptr<simcore::hepmc::HepMCEvent> hepmc_event =
      reader_.readNextEvent();

  if (hepmc_event != nullptr) {
    // One G4 vertex per HepMC production vertex, in order of first use
    std::vector<std::pair<int, G4PrimaryVertex*>> vertices;

    const auto& particles = hepmc_event->getParticles();
    for (const auto& particle : particles) {
      const int vtx_id = particle->getProductionVertexId();
      auto it =
          std::find_if(vertices.begin(), vertices.end(),
                       [vtx_id](const auto& v) { return v.first == vtx_id; });
      G4PrimaryVertex* vertex{nullptr};
      if (it == vertices.end()) {
        // HepMC3 lengths are mm after the reader converts units
        const HepMC3::FourVector pos = particle->getProductionPosition();
        // This is a raw pointer, and GEANT4 will delete it
        vertex = new G4PrimaryVertex(pos.x() + vertex_[0], pos.y() + vertex_[1],
                                     pos.z() + vertex_[2], pos.t() / c_light);
        // vertex weights multiply, so only the first carries the event weight
        if (vertices.empty()) vertex->SetWeight(hepmc_event->getEventWeight());
        vertices.emplace_back(vtx_id, vertex);
      } else {
        vertex = it->second;
      }

      // Create a primary particle for Geant4
      // This is a raw pointer, and GEANT4 will delete it
      G4PrimaryParticle* primary = new G4PrimaryParticle();

      primary->SetPDGcode(particle->getPdgId());

      // Set the primary particle's momentum
      // HepMC3 uses GeV by default, which is what Geant4 expects
      primary->Set4Momentum(particle->getMomentum(0) * GeV,  // px
                            particle->getMomentum(1) * GeV,  // py
                            particle->getMomentum(2) * GeV,  // pz
                            particle->getMomentum(3) * GeV   // E
      );

      auto primary_info = std::make_unique<UserPrimaryParticleInformation>();
      primary_info->setHepEvtStatus(particle->getStatus());
      primary->SetUserInformation(primary_info.release());

      vertex->SetPrimary(primary);
    }

    // Apply beam spot smearing if configured, same shift for all vertices
    if (useBeamspot() && !vertices.empty()) {
      G4PrimaryVertex* first = vertices.front().second;
      const G4ThreeVector before = first->GetPosition();
      smearBeamspot(first);
      const G4ThreeVector shift = first->GetPosition() - before;
      for (std::size_t i = 1; i < vertices.size(); ++i) {
        auto* vertex = vertices[i].second;
        vertex->SetPosition(vertex->GetX0() + shift.x(),
                            vertex->GetY0() + shift.y(),
                            vertex->GetZ0() + shift.z());
      }
    }

    for (auto& [vtx_id, vertex] : vertices) anEvent->AddPrimaryVertex(vertex);

  } else {
    ldmx_log(error) << "Ran out of input events so run will be aborted!";
    G4RunManager::GetRunManager()->AbortRun(true);
    anEvent->SetEventAborted();
  }
}

void HepMCPrimaryGenerator::RecordConfig(const std::string& id,
                                         ldmx::RunHeader& rh) {
  rh.setStringParameter(id + " Class",
                        "simcore::generators::HepMCPrimaryGenerator");
  rh.setStringParameter(id + " HepMC File", file_path_);
}

}  // namespace generators
}  // namespace simcore

DECLARE_GENERATOR(simcore::generators::HepMCPrimaryGenerator)

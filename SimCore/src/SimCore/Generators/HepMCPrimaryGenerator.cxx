#include "SimCore/Generators/HepMCPrimaryGenerator.h"

namespace simcore {
namespace generators {

HepMCPrimaryGenerator::HepMCPrimaryGenerator(
    const std::string& name, const framework::config::Parameters& parameters)
    : PrimaryGenerator(name, parameters),
      file_path_{parameters.get<std::string>("file_path")},
      reader_{file_path_},
      vertex_{parameters.get<std::vector<double>>("vertex")} {}

void HepMCPrimaryGenerator::GeneratePrimaryVertex(G4Event* anEvent) {
  std::unique_ptr<simcore::hepmc::HepMCEvent> hepmc_event = reader_.readNextEvent();

  if (hepmc_event != nullptr) {
    // Create a primary vertex for the Geant4 event
    // This is a raw pointer, and GEANT4 will delete it
    G4PrimaryVertex* vertex = new G4PrimaryVertex();
    vertex->SetPosition(hepmc_event->getVertex()[0] + vertex_[0],
                        hepmc_event->getVertex()[1] + vertex_[1],
                        hepmc_event->getVertex()[2] + vertex_[2]);
    vertex->SetT0(hepmc_event->getVertexTime());
    vertex->SetWeight(hepmc_event->getEventWeight());

    // Map to track parent-daughter relationships
    std::map<std::shared_ptr<HepMC3::GenParticle>, G4PrimaryParticle*> particle_map;

    const auto& particles = hepmc_event->getParticles();
    for (const auto& particle : particles) {
      // Create a primary particle for Geant4
      // This is a raw pointer, and GEANT4 will delete it
      G4PrimaryParticle* primary = new G4PrimaryParticle();

      primary->SetPDGcode(particle->getPdgId());

      // Set the primary particle's momentum
      // HepMC3 uses GeV by default, which is what Geant4 expects
      primary->Set4Momentum(
          particle->getMomentum(0) * GeV,  // px
          particle->getMomentum(1) * GeV,  // py
          particle->getMomentum(2) * GeV,  // pz
          particle->getMomentum(3) * GeV   // E
      );

      auto primary_info = std::make_unique<UserPrimaryParticleInformation>();
      primary_info->setHepEvtStatus(particle->getStatus());
      primary->SetUserInformation(primary_info.release());

      // Store the particle in the map for potential parent-daughter relationships
      particle_map[particle->getGenParticle()] = primary;

      // Add the particle to the vertex
      // In HepMC, we add all final state particles directly to the vertex
      // since we've already filtered for final state particles
      vertex->SetPrimary(primary);
    }

    anEvent->AddPrimaryVertex(vertex);

    // Apply beam spot smearing if configured for this generator
    if (useBeamspot()) {
      smearBeamspot(vertex);
    }

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

#include "SimCore/Generators/LHEPrimaryGenerator.h"

namespace simcore {
namespace generators {

LHEPrimaryGenerator::LHEPrimaryGenerator(
    const std::string& name, const framework::config::Parameters& parameters)
    : PrimaryGenerator(name, parameters),
      file_path_{parameters.get<std::string>("filePath")},
      reader_{file_path_},
      vertex_{parameters.get<std::vector<double>>("vertex")} {}

void LHEPrimaryGenerator::GeneratePrimaryVertex(G4Event* anEvent) {
  std::unique_ptr<simcore::lhe::LHEEvent> lheEvent = reader_.readNextEvent();

  if (lheEvent != nullptr) {
    // Create a primary vertex for the Geant4 event
    // This is a raw pointer, and GEANT4 will delete it
    G4PrimaryVertex* vertex = new G4PrimaryVertex();
    vertex->SetPosition(lheEvent->getVertex()[0] + vertex_[0],
                        lheEvent->getVertex()[1] + vertex_[1],
                        lheEvent->getVertex()[2] + vertex_[2]);
    vertex->SetWeight(lheEvent->getEventWeight());

    std::map<simcore::lhe::LHEParticle*, G4PrimaryParticle*> particleMap;

    const auto& particles = lheEvent->getParticles();
    for (const auto& particle : particles) {
      // Check if the particle has a valid, outgoing particle status
      if (particle->getStatus() > 0) {
        // Create a primary particle for the Geant4
        // This is a raw pointer, and GEANT4 will delete it
        G4PrimaryParticle* primary = new G4PrimaryParticle();
        // Tungsten ion in the LHE files
        // TODO: can this never be +623?
        if (particle->getPdgId() == -623) {
          G4ParticleDefinition* tungstenIonDef =
              G4IonTable::GetIonTable()->GetIon(74, 184, 0.);
          if (tungstenIonDef != nullptr) {
            primary->SetParticleDefinition(tungstenIonDef);
          } else {
            EXCEPTION_RAISE("EventGenerator",
                            "Failed to find particle definition for W ion.");
          }
        } else {
          primary->SetPDGcode(particle->getPdgId());
        }

        // TODO: should we not require that the primary is an electron?
        // for example the WAB LHE events will have the wide photon as primary

        primary->Set4Momentum(
            particle->getMomentum(0) * GeV, particle->getMomentum(1) * GeV,
            particle->getMomentum(2) * GeV, particle->getMomentum(3) * GeV);
        primary->SetProperTime(particle->getLifetime() * nanosecond);

        auto primary_info = std::make_unique<UserPrimaryParticleInformation>();
        primary_info->setHepEvtStatus(particle->getStatus());
        primary->SetUserInformation(primary_info.release());

        particleMap[particle.get()] = primary;

        /*
         * Assign primary as daughter but only if the mother is not a DOC
         * particle->
         */
        if (particle->getMotherParticle(0) != nullptr &&
            particle->getMotherParticle(0)->getStatus() > 0) {
          G4PrimaryParticle* primary_mom =
              particleMap[particle->getMotherParticle(0)];
          if (primary_mom != nullptr) {
            primary_mom->SetDaughter(primary);
          }
        } else {
          vertex->SetPrimary(primary);
        }
      }  // end condition for valid, outgoing particle status
    }

    anEvent->AddPrimaryVertex(vertex);

  } else {
    ldmx_log(info) << "Ran out of input events so run will be aborted!";
    G4RunManager::GetRunManager()->AbortRun(true);
    anEvent->SetEventAborted();
  }
}

void LHEPrimaryGenerator::RecordConfig(const std::string& id,
                                       ldmx::RunHeader& rh) {
  rh.setStringParameter(id + " Class",
                        "simcore::generators::LHEPrimaryGenerator");
  rh.setStringParameter(id + " LHE File", file_path_);
}

}  // namespace generators
}  // namespace simcore

DECLARE_GENERATOR(simcore::generators::LHEPrimaryGenerator)
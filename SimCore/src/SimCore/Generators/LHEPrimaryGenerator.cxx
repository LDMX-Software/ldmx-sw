#include "SimCore/Generators/LHEPrimaryGenerator.h"

namespace simcore {
namespace generators {

LHEPrimaryGenerator::LHEPrimaryGenerator(
    const std::string& name, const framework::config::Parameters& parameters)
    : PrimaryGenerator(name, parameters) {
  file_path_ = parameters.getParameter<std::string>("filePath");
  reader_ = std::make_unique<simcore::lhe::LHEReader>(file_path_);

  // // Validate the reader
  // if (!reader_) {
  //   EXCEPTION_RAISE("EventGenerator","LHEReader failed to initialize with
  //   file path: " + file_path_);
  // }
}

void LHEPrimaryGenerator::GeneratePrimaryVertex(G4Event* anEvent) {
  std::unique_ptr<simcore::lhe::LHEEvent> lheEvent = reader_->readNextEvent();

  if (lheEvent != nullptr) {
    auto vertex = std::make_unique<G4PrimaryVertex>();
    vertex->SetPosition(lheEvent->getVertex()[0], lheEvent->getVertex()[1],
                        lheEvent->getVertex()[2]);
    vertex->SetWeight(lheEvent->getXWGTUP());

    std::map<simcore::lhe::LHEParticle*, G4PrimaryParticle*> particleMap;

    const auto& particles = lheEvent->getParticles();
    for (const auto& particle : particles) {
      if (particle->getISTUP() > 0) {
        auto primary = std::make_unique<G4PrimaryParticle>();
        if (particle->getIDUP() == -623) { /* Tungsten ion */
          G4ParticleDefinition* tungstenIonDef =
              G4IonTable::GetIonTable()->GetIon(74, 184, 0.);
          if (tungstenIonDef != nullptr) {
            primary->SetParticleDefinition(tungstenIonDef);
          } else {
            EXCEPTION_RAISE("EventGenerator",
                            "Failed to find particle definition for W ion.");
          }
        } else {
          primary->SetPDGcode(particle->getIDUP());
        }

        primary->Set4Momentum(
            particle->getPUP(0) * GeV, particle->getPUP(1) * GeV,
            particle->getPUP(2) * GeV, particle->getPUP(3) * GeV);
        primary->SetProperTime(particle->getVTIMUP() * nanosecond);

        auto primaryInfo = std::make_unique<UserPrimaryParticleInformation>();
        primaryInfo->setHepEvtStatus(particle->getISTUP());
        primary->SetUserInformation(primaryInfo.release());

        particleMap[particle.get()] = primary.get();

        /*
         * Assign primary as daughter but only if the mother is not a DOC
         * particle->
         */
        if (particle->getMother(0) != nullptr &&
            particle->getMother(0)->getISTUP() > 0) {
          G4PrimaryParticle* primaryMom = particleMap[particle->getMother(0)];
          if (primaryMom != nullptr) {
            primaryMom->SetDaughter(primary.release());
          }
        } else {
          vertex->SetPrimary(primary.release());
        }
      }
    }

    anEvent->AddPrimaryVertex(vertex.release());

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
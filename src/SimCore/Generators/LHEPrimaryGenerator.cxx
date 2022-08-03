/**
 * @file LHEPrimaryGenerator.cxx
 * @brief Implementation file for LHEPrimaryGenerator
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 * @author Tom Eichlersmith, University of Minnesota
 */

#include "SimCore/Generators/LHEPrimaryGenerator.h"

// Geant4
#include "G4Event.hh"
#include "G4IonTable.hh"
#include "G4PhysicalConstants.hh"
#include "G4RunManager.hh"
#include "G4SystemOfUnits.hh"

// LDMX
#include "Framework/Configure/Parameters.h"
#include "Framework/Exception/Exception.h"
#include "SimCore/UserPrimaryParticleInformation.h"

namespace simcore {
namespace generators {

LHEPrimaryGenerator::LHEPrimaryGenerator(const std::string& name,
                                         const framework::config::Parameters& parameters)
    : PrimaryGenerator(name, parameters) {
  std::string filePath = parameters_.getParameter<std::string>("filePath");
  reader_ = new simcore::lhe::LHEReader(filePath);
}

LHEPrimaryGenerator::~LHEPrimaryGenerator() { delete reader_; }

void LHEPrimaryGenerator::GeneratePrimaryVertex(G4Event* anEvent) {
  simcore::lhe::LHEEvent* lheEvent = reader_->readNextEvent();

  if (lheEvent != NULL) {
    G4PrimaryVertex* vertex = new G4PrimaryVertex();
    vertex->SetPosition(lheEvent->getVertex()[0], lheEvent->getVertex()[1],
                        lheEvent->getVertex()[2]);
    vertex->SetWeight(lheEvent->getXWGTUP());

    std::map<simcore::lhe::LHEParticle*, G4PrimaryParticle*> particleMap;

    int particleIndex = 0;
    const std::vector<simcore::lhe::LHEParticle*>& particles = lheEvent->getParticles();
    for (auto it = particles.begin();
         it != particles.end(); it++) {
      simcore::lhe::LHEParticle* particle = (*it);

      if (particle->getISTUP() > 0) {
        G4PrimaryParticle* primary = new G4PrimaryParticle();
        if (particle->getIDUP() == -623) { /* Tungsten ion */
          G4ParticleDefinition* tungstenIonDef =
              G4IonTable::GetIonTable()->GetIon(74, 184, 0.);
          if (tungstenIonDef != NULL) {
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

        UserPrimaryParticleInformation* primaryInfo =
            new UserPrimaryParticleInformation();
        primaryInfo->setHepEvtStatus(particle->getISTUP());
        primary->SetUserInformation(primaryInfo);

        particleMap[particle] = primary;

        /*
         * Assign primary as daughter but only if the mother is not a DOC
         * particle.
         */
        if (particle->getMother(0) != NULL &&
            particle->getMother(0)->getISTUP() > 0) {
          G4PrimaryParticle* primaryMom = particleMap[particle->getMother(0)];
          if (primaryMom != NULL) {
            primaryMom->SetDaughter(primary);
          }
        } else {
          vertex->SetPrimary(primary);
        }
      }

      ++particleIndex;
    }

    anEvent->AddPrimaryVertex(vertex);

  } else {
    std::cout << "[ LHEPrimaryGenerator ] : Ran out of input events so run "
                 "will be aborted!"
              << std::endl;
    G4RunManager::GetRunManager()->AbortRun(true);
    anEvent->SetEventAborted();
  }

  delete lheEvent;
}

}  // namespace generators
}  // namespace simcore

DECLARE_GENERATOR(simcore::generators, LHEPrimaryGenerator)

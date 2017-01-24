#include "SimApplication/LHEPrimaryGenerator.h"

// Geant4
#include "G4Event.hh"
#include "G4IonTable.hh"

// LDMX
#include "SimApplication/UserPrimaryParticleInformation.h"

// Geant4
#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"

namespace ldmx {

LHEPrimaryGenerator::LHEPrimaryGenerator(LHEReader* theReader)
    : reader_(theReader) {
}

LHEPrimaryGenerator::~LHEPrimaryGenerator() {
    delete reader_;
}

void LHEPrimaryGenerator::GeneratePrimaryVertex(G4Event* anEvent) {

    std::cout << "Reading next LHE event ..." << std::endl;

    LHEEvent* lheEvent = reader_->readNextEvent();

    if (lheEvent != NULL) {

        G4PrimaryVertex* vertex = new G4PrimaryVertex();
        vertex->SetPosition(0, 0, 0);
        vertex->SetWeight(lheEvent->getXWGTUP());

        std::map<LHEParticle*, G4PrimaryParticle*> particleMap;

        std::cout << std::endl;

        int particleIndex = 0;
        const std::vector<LHEParticle*>& particles = lheEvent->getParticles();
        for (std::vector<LHEParticle*>::const_iterator it = particles.begin();
                it != particles.end(); it++) {

            std::cout << "Processing input LHE particle[" << particleIndex << "]" << std::endl;

            LHEParticle* particle = (*it);

            if (particle->getISTUP() > 0) {

                G4PrimaryParticle* primary = new G4PrimaryParticle();
                if (particle->getIDUP() == -623) { /* Tungsten ion */
                    G4ParticleDefinition* tungstenIonDef = G4IonTable::GetIonTable()->GetIon(74, 184, 0.);
                    if (tungstenIonDef != NULL) {
                        primary->SetParticleDefinition(tungstenIonDef);
                    } else {
                        G4Exception("LHEPrimaryGenerator::GeneratePrimaryVertex",
                                "EventGenerationError",
                                FatalException,
                                "Failed to find particle definition for W ion.");
                    }
                } else {
                    primary->SetPDGcode(particle->getIDUP());
                }

                primary->SetMomentum(particle->getPUP(0) * GeV,
                        particle->getPUP(1) * GeV,
                        particle->getPUP(2) * GeV);
                primary->SetMass(particle->getPUP(4) * GeV);
                primary->SetProperTime(particle->getVTIMUP() * nanosecond);

                UserPrimaryParticleInformation* primaryInfo = new UserPrimaryParticleInformation();
                primaryInfo->setHepEvtStatus(particle->getISTUP());
                primary->SetUserInformation(primaryInfo);

                particleMap[particle] = primary;

                /*
                 * Assign primary as daughter but only if the mother is not a DOC particle.
                 */
                if (particle->getMother(0) != NULL && particle->getMother(0)->getISTUP() > 0) {
                    G4PrimaryParticle* primaryMom = particleMap[particle->getMother(0)];
                    if (primaryMom != NULL) {
                        primaryMom->SetDaughter(primary);
                        std::cout << "  Added dau to mother primary" << std::endl;
                    }
                } else {
                    vertex->SetPrimary(primary);
                    std::cout << "  Added primary to vertex" << std::endl;
                }

                primary->Print();

            } else {
                std::cout << "  Skipping LHE particle with status " << particle->getISTUP() << std::endl;
            }

            std::cout << std::endl;

            ++particleIndex;
        }

        anEvent->AddPrimaryVertex(vertex);

    } else {
        G4Exception("LHEPrimaryGenerator::GeneratePrimaryVertex",
                "EventUnderflow",
                RunMustBeAborted,
                "No more LHE events found in input file.");
    }

    delete lheEvent;
}

}

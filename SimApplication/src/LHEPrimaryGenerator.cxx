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

    LHEPrimaryGenerator::LHEPrimaryGenerator(LHEReader* theReader) :
            reader_(theReader) {
    }

    LHEPrimaryGenerator::~LHEPrimaryGenerator() {
        delete reader_;
    }

    void LHEPrimaryGenerator::GeneratePrimaryVertex(G4Event* anEvent) {

        LHEEvent* lheEvent = reader_->readNextEvent();

        if (lheEvent != NULL) {

            G4PrimaryVertex* vertex = new G4PrimaryVertex();
<<<<<<< HEAD
            vertex->SetPosition(lheEvent->getVertex()[0],lheEvent->getVertex()[1],lheEvent->getVertex()[2]);
=======
            vertex->SetPosition(0, 0, 0);
>>>>>>> 8b6eac63b072f76349363b0a0ec1b1d9103c12f8
            vertex->SetWeight(lheEvent->getXWGTUP());

            std::map<LHEParticle*, G4PrimaryParticle*> particleMap;

            int particleIndex = 0;
            const std::vector<LHEParticle*>& particles = lheEvent->getParticles();
            for (std::vector<LHEParticle*>::const_iterator it = particles.begin(); it != particles.end(); it++) {

                LHEParticle* particle = (*it);

                if (particle->getISTUP() > 0) {

                    G4PrimaryParticle* primary = new G4PrimaryParticle();
                    if (particle->getIDUP() == -623) { /* Tungsten ion */
                        G4ParticleDefinition* tungstenIonDef = G4IonTable::GetIonTable()->GetIon(74, 184, 0.);
                        if (tungstenIonDef != NULL) {
                            primary->SetParticleDefinition(tungstenIonDef);
                        } else {
<<<<<<< HEAD
                            G4Exception("LHEPrimaryGenerator::GeneratePrimaryVertex", 
                                        "EventGenerationError", FatalException, 
                                        "Failed to find particle definition for W ion.");
=======
                            G4Exception("LHEPrimaryGenerator::GeneratePrimaryVertex", "EventGenerationError", FatalException, "Failed to find particle definition for W ion.");
>>>>>>> 8b6eac63b072f76349363b0a0ec1b1d9103c12f8
                        }
                    } else {
                        primary->SetPDGcode(particle->getIDUP());
                    }

<<<<<<< HEAD
                    primary->Set4Momentum(particle->getPUP(0) * GeV, 
                                          particle->getPUP(1) * GeV, 
                                          particle->getPUP(2) * GeV, 
                                          particle->getPUP(3) * GeV);
=======
                    primary->Set4Momentum(particle->getPUP(0) * GeV, particle->getPUP(1) * GeV, particle->getPUP(2) * GeV, particle->getPUP(3) * GeV);
>>>>>>> 8b6eac63b072f76349363b0a0ec1b1d9103c12f8
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
                        }
                    } else {
                        vertex->SetPrimary(primary);
                    }

<<<<<<< HEAD
                } 
=======
                    primary->Print();

                } else {
                }

                std::cout << std::endl;
>>>>>>> 8b6eac63b072f76349363b0a0ec1b1d9103c12f8

                ++particleIndex;
            }

            anEvent->AddPrimaryVertex(vertex);

        } else {
            std::cout << "[ LHEPrimaryGenerator ] : Ran out of input events so run will be aborted!" << std::endl;
            G4RunManager::GetRunManager()->AbortRun(true);
            anEvent->SetEventAborted();
        }

        delete lheEvent;
    }

}

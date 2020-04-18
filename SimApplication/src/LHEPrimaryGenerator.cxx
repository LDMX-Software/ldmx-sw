/**
 * @file LHEPrimaryGenerator.cxx
 * @brief Implementation file for LHEPrimaryGenerator
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 * @author Tom Eichlersmith, University of Minnesota
 */

#include "SimApplication/LHEPrimaryGenerator.h"

// Geant4
#include "G4Event.hh"
#include "G4IonTable.hh"
#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"
#include "G4RunManager.hh"

// LDMX
#include "SimApplication/UserPrimaryParticleInformation.h"
#include "Framework/Parameters.h"
#include "Exception/Exception.h"

namespace ldmx {

    LHEPrimaryGenerator::LHEPrimaryGenerator(const std::string& name , Parameters& parameters)
        : PrimaryGenerator( name , parameters ) {

        std::string filePath = parameters_.getParameter< std::string >( "filePath" );
        reader_ = new LHEReader( filePath );

    }

    LHEPrimaryGenerator::~LHEPrimaryGenerator() {
        delete reader_;
    }

    void LHEPrimaryGenerator::GeneratePrimaryVertex(G4Event* anEvent) {

        LHEEvent* lheEvent = reader_->readNextEvent();

        if (lheEvent != NULL) {

            G4PrimaryVertex* vertex = new G4PrimaryVertex();
            vertex->SetPosition(lheEvent->getVertex()[0],lheEvent->getVertex()[1],lheEvent->getVertex()[2]);
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
                            EXCEPTION_RAISE( "EventGenerator" ,
                                    "Failed to find particle definition for W ion." );
                        }
                    } else {
                        primary->SetPDGcode(particle->getIDUP());
                    }

                    primary->Set4Momentum(particle->getPUP(0) * GeV, 
                                          particle->getPUP(1) * GeV, 
                                          particle->getPUP(2) * GeV, 
                                          particle->getPUP(3) * GeV);
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

                } 

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

DECLARE_GENERATOR( ldmx , LHEPrimaryGenerator )

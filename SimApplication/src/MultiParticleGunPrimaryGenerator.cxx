#include "SimApplication/MultiParticleGunPrimaryGenerator.h"

// Geant4
#include "G4RunManager.hh"
#include "G4Event.hh"
#include "G4IonTable.hh"

// LDMX
#include "SimApplication/UserPrimaryParticleInformation.h"
#include "Event/SimParticle.h"
#include "Event/EventConstants.h"

// Geant4
#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"

namespace ldmx {

    MultiParticleGunPrimaryGenerator::MultiParticleGunPrimaryGenerator() {
        random_ = new TRandom();
    }

    MultiParticleGunPrimaryGenerator::~MultiParticleGunPrimaryGenerator() {
    }

    void MultiParticleGunPrimaryGenerator::GeneratePrimaryVertex(G4Event* anEvent) {

        std::cout << "Generating some gun events! ... " << std::endl;
        std::cout << "Particle energy = " << PrimaryGeneratorMessenger::getMPGParticleEnergy() << std::endl;

        // get the random number of interactions to generate
        int nInteractions = PrimaryGeneratorMessenger::getMPGnInteractions();
        int nInteractions_Poisson = random_->Poisson(nInteractions);
        std::cout << "n Incoming = " << nInteractions << ", n random = " << random_->Poisson(nInteractions) << std::endl;

        // if (nInteractions_Poisson == 0) G4RunManager::GetRunManager()->AbortEvent();

        // make a for loop
        for (int i = 0; i < nInteractions; ++i){

            G4PrimaryVertex* curvertex = new G4PrimaryVertex();
            curvertex->SetPosition(0. * mm,0. * mm,-10. * mm);
            curvertex->SetWeight(1.);
            
            G4PrimaryParticle* primary = new G4PrimaryParticle();

            primary->SetPDGcode(11);
            primary->SetMomentum(0. * MeV, 0. * MeV, 3967.2 * MeV);
            primary->SetMass(511. * MeV);

            UserPrimaryParticleInformation* primaryInfo = new UserPrimaryParticleInformation();
            primaryInfo->setHepEvtStatus(1.);
            primary->SetUserInformation(primaryInfo);

            curvertex->SetPrimary(primary);    
            anEvent->AddPrimaryVertex(curvertex);

        }      
    }

}

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

    MultiParticleGunPrimaryGenerator::MultiParticleGunPrimaryGenerator():
        random_ (new TRandom){
    }

    MultiParticleGunPrimaryGenerator::~MultiParticleGunPrimaryGenerator() {
    }

    void MultiParticleGunPrimaryGenerator::GeneratePrimaryVertex(G4Event* anEvent) {

        int cur_mpg_pdgid = PrimaryGeneratorMessenger::getMPGPdgId();
        G4ThreeVector cur_mpg_vertex = PrimaryGeneratorMessenger::getMPGVertex();
        G4ThreeVector cur_mpg_momentum = PrimaryGeneratorMessenger::getMPGMomentum();

        // current number of vertices in the event! 
        std::cout << "[MultiParticleGunPrimaryGenerator::GeneratePrimaryVertex] number of vertices in the event! " << anEvent->GetNumberOfPrimaryVertex() << std::endl;
        int curNVertices = anEvent->GetNumberOfPrimaryVertex();

        double nInteractionsInput = PrimaryGeneratorMessenger::getMPGNParticles();
        int nInteractions = nInteractionsInput;
        if (PrimaryGeneratorMessenger::getEnablePoisson()){ 
			nInteractions = 0;
			while (nInteractions == 0){ // keep generating a random poisson until > 0, no point in generator 0 vertices...
        		nInteractions = random_->Poisson(nInteractionsInput);
        	}
        }

	    std::cout << "[MultiParticleGunPrimaryGenerator::GeneratePrimaryVertex] number of interactions = " << nInteractions << "," << nInteractionsInput << std::endl;

        // make a for loop
        for (int i = 0; i < (nInteractions-curNVertices); ++i){

            G4PrimaryVertex* curvertex = new G4PrimaryVertex(cur_mpg_vertex,0.); //second input is t0
            // curvertex->SetPosition(0. * mm,0. * mm,-10. * mm);
            curvertex->SetWeight(1.);
            
            G4PrimaryParticle* primary = new G4PrimaryParticle(cur_mpg_pdgid,cur_mpg_momentum.x(),cur_mpg_momentum.y(),cur_mpg_momentum.z());

            // primary->SetPDGcode(11);
            // primary->SetMomentum(0. * MeV, 0. * MeV, 3967.2 * MeV);
            // primary->SetMass(511. * MeV);

            UserPrimaryParticleInformation* primaryInfo = new UserPrimaryParticleInformation();
            primaryInfo->setHepEvtStatus(1.);
            primary->SetUserInformation(primaryInfo);

            curvertex->SetPrimary(primary);    
            anEvent->AddPrimaryVertex(curvertex);

        }      
        // std::cout << "after mpg, number of vertices in the event! " << anEvent->GetNumberOfPrimaryVertex() << std::endl;

    }
}

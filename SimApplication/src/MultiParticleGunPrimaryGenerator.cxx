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
        random_ (new TRandom),
        mpg_enablePoisson_ (false),
        mpg_pdgId_ (99999),
        mpg_nparticles_ (1.) {
    }

    MultiParticleGunPrimaryGenerator::~MultiParticleGunPrimaryGenerator() {
    }

    void MultiParticleGunPrimaryGenerator::GeneratePrimaryVertex(G4Event* anEvent) {

        int cur_mpg_pdgid = mpg_pdgId_;
        G4ThreeVector cur_mpg_vertex = mpg_vertex_;
        G4ThreeVector cur_mpg_momentum = mpg_momentum_;

        // current number of vertices in the event! 
        int curNVertices = anEvent->GetNumberOfPrimaryVertex();

        double nInteractionsInput = mpg_nparticles_;
        int nInteractions = nInteractionsInput;
        if (mpg_enablePoisson_){ 
			nInteractions = 0;
			while (nInteractions == 0){ // keep generating a random poisson until > 0, no point in generator 0 vertices...
        		nInteractions = random_->Poisson(nInteractionsInput);
        	}
        }

        // make a for loop
        for (int i = 0; i < (nInteractions-curNVertices); ++i){

            G4PrimaryVertex* curvertex = new G4PrimaryVertex(cur_mpg_vertex,0.); //second input is t0
            // curvertex->SetPosition(0. * mm,0. * mm,-10. * mm);
            curvertex->SetWeight(1.);
            
            G4PrimaryParticle* primary = new G4PrimaryParticle(cur_mpg_pdgid,cur_mpg_momentum.x(),cur_mpg_momentum.y(),cur_mpg_momentum.z());

            UserPrimaryParticleInformation* primaryInfo = new UserPrimaryParticleInformation();
            primaryInfo->setHepEvtStatus(1.);
            primary->SetUserInformation(primaryInfo);

            curvertex->SetPrimary(primary);    
            anEvent->AddPrimaryVertex(curvertex);

        }      

    }
}

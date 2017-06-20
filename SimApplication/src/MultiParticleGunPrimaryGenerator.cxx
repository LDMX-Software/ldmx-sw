/**
 * @file MultiParticleGunPrimaryGenerator.cxx
 * @brief Class for generating an event using multiple particles.
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 * @author Nhan Tran, FNAL
 */

#include "SimApplication/MultiParticleGunPrimaryGenerator.h"

namespace ldmx {

    MultiParticleGunPrimaryGenerator::MultiParticleGunPrimaryGenerator():
        random_ (new TRandom) {
    }

    MultiParticleGunPrimaryGenerator::~MultiParticleGunPrimaryGenerator() {
    }

    void MultiParticleGunPrimaryGenerator::GeneratePrimaryVertex(G4Event* anEvent) {

        int cur_mpg_pdgid = mpgPdgID_;
        G4ThreeVector cur_mpg_vertex = mpgVertex_;
        G4ThreeVector cur_mpg_momentum = mpgMomentum_;

        // current number of vertices in the event! 
        int curNVertices = anEvent->GetNumberOfPrimaryVertex();

        double nInteractionsInput = mpgNParticles_;
        int nInteractions = nInteractionsInput;
        if (mpgEnablePoisson_){ 
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

#include "SimApplication/PrimaryGeneratorAction.h"
#include "SimApplication/UserPrimaryParticleInformation.h"

// Geant4
#include "G4Event.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "G4SystemOfUnits.hh"

namespace ldmx {

PrimaryGeneratorAction::PrimaryGeneratorAction()
 : G4VUserPrimaryGeneratorAction(), generator_(new G4ParticleGun) {
}

PrimaryGeneratorAction::~PrimaryGeneratorAction() {
    delete generator_;
}

void PrimaryGeneratorAction::setPrimaryGenerator(G4VPrimaryGenerator* aGenerator) {
    PrimaryGeneratorAction::generator_ = aGenerator;
}

void PrimaryGeneratorAction::GeneratePrimaries(G4Event* event) {
    generator_->GeneratePrimaryVertex(event);
 
    // automatically setting genStatus to 1 for particle gun primaries
    if (dynamic_cast<G4ParticleGun*>(generator_) != NULL){
        
        int nPV = event->GetNumberOfPrimaryVertex();
        for (int iPV = 0; iPV < nPV; ++iPV){
            G4PrimaryVertex* curPV =  event->GetPrimaryVertex(iPV);
            int nPar = curPV->GetNumberOfParticle();            
            for (int iPar = 0; iPar < nPar; ++iPar){
                UserPrimaryParticleInformation* primaryInfo = new UserPrimaryParticleInformation();
                primaryInfo->setHepEvtStatus(1);
                curPV->GetPrimary(iPar)->SetUserInformation(primaryInfo);
            }
        }
               
    }

    // Activate the plugin manager hook.        
    pluginManager_->generatePrimary(event);
}

}

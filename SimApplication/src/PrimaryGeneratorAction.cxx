/**
 * @file PrimaryGeneratorAction.cxx
 * @brief Class implementing the Geant4 primary generator action
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#include "SimApplication/PrimaryGeneratorAction.h"

namespace ldmx {

    PrimaryGeneratorAction::PrimaryGeneratorAction() :
        G4VUserPrimaryGeneratorAction(), 
        random_(new TRandom) {
        random_->SetSeed( CLHEP::HepRandom::getTheSeed() ); 
        generator_.push_back(new G4ParticleGun());
    }

    PrimaryGeneratorAction::~PrimaryGeneratorAction() {
    }

    void PrimaryGeneratorAction::setPrimaryGenerator(G4VPrimaryGenerator* aGenerator) {
       
        // The other generators don't play nice with G4ParticleGun, so
        // if there is already a generator and it's the G4ParticleGun just clear the vector 
        bool clearGeneratorVector = false;
        unsigned int ngens = generator_.size();
        for (unsigned int i = 0; i < ngens; ++i){
            if (dynamic_cast<G4ParticleGun*>(generator_[i]) !=  NULL) {
                clearGeneratorVector = true;
            }
        }
        if (clearGeneratorVector) generator_.clear();

        generator_.push_back(aGenerator);

        if ((dynamic_cast<MultiParticleGunPrimaryGenerator*>(aGenerator)) != NULL){
            indexMpg_ = ((int) generator_.size()) - 1;
        }
        if ((dynamic_cast<RootPrimaryGenerator*>(aGenerator)) != NULL){
            indexRpg_ = ((int) generator_.size()) - 1;
        }        
    }

    void PrimaryGeneratorAction::GeneratePrimaries(G4Event* event) {
        
        unsigned int ngens = generator_.size();
        for (unsigned int i = 0; i < ngens; ++i) {
            generator_[i]->GeneratePrimaryVertex(event);
            
            // automatically setting genStatus to 1 for particle gun primaries
            if (dynamic_cast<G4ParticleGun*>(generator_[i]) !=  NULL) {
                int nPV = event->GetNumberOfPrimaryVertex();
                for (int iPV = 0; iPV < nPV; ++iPV) {
                    G4PrimaryVertex* curPV = event->GetPrimaryVertex(iPV);
                    int nPar = curPV->GetNumberOfParticle();
                    for (int iPar = 0; iPar < nPar; ++iPar) {
                        UserPrimaryParticleInformation* primaryInfo = new UserPrimaryParticleInformation();
                        primaryInfo->setHepEvtStatus(1);
                        curPV->GetPrimary(iPar)->SetUserInformation(primaryInfo);
                    }
                }
            }
        }
            
        // Activate the plugin manager hook.  
        if (event->GetNumberOfPrimaryVertex() > 0) {
            if (useBeamspot_) smearingBeamspot(event);        
            pluginManager_->generatePrimary(event);
        }

    }

    void PrimaryGeneratorAction::smearingBeamspot(G4Event* event) {
        
        double IPWidthX = beamspotXSize_/2.;
        double IPWidthY = beamspotYSize_/2.;
        double IPWidthZ = beamspotZSize_/2.;

        int nPV = event->GetNumberOfPrimaryVertex();
        for (int iPV = 0; iPV < nPV; ++iPV) {
            G4PrimaryVertex* curPV = event->GetPrimaryVertex(iPV);
            double x0_i = curPV->GetX0();
            double y0_i = curPV->GetY0();
            double z0_i = curPV->GetZ0();
            double x0_f = random_->Uniform( x0_i - IPWidthX, x0_i + IPWidthX );
            double y0_f = random_->Uniform( y0_i - IPWidthY, y0_i + IPWidthY );
            double z0_f = random_->Uniform( z0_i - IPWidthZ, z0_i + IPWidthZ );            
            curPV->SetPosition( x0_f, y0_f, z0_f );
        }        

    }
}

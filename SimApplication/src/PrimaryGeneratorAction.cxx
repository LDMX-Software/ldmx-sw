#include "SimApplication/PrimaryGeneratorAction.h"
#include "SimApplication/UserPrimaryParticleInformation.h"
#include "SimApplication/PrimaryGeneratorMessenger.h"

// Geant4
#include "G4RunManager.hh"
#include "G4Event.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "G4SystemOfUnits.hh"

namespace ldmx {

    PrimaryGeneratorAction::PrimaryGeneratorAction() :
            G4VUserPrimaryGeneratorAction(), 
            // generator_(new G4ParticleGun), 
            random_(new TRandom),
            useBeamspot_(false),
            beamspotXSize_(20.),
            beamspotYSize_(10.)  {
      generator_.push_back(new G4ParticleGun());
    }

    PrimaryGeneratorAction::~PrimaryGeneratorAction() {
        // delete generator_;
    }

    void PrimaryGeneratorAction::setPrimaryGenerator(G4VPrimaryGenerator* aGenerator) {
       
        // the other generators don't place nice with G4ParticleGun, so
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
        
    }

    void PrimaryGeneratorAction::GeneratePrimaries(G4Event* event) {
        
        unsigned int ngens = generator_.size();
        for (unsigned int i = 0; i < ngens; ++i){
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
        // std::cout << "[PrimaryGeneratorAction::GeneratePrimaries] useBeamspot_ = " << useBeamspot_ << ", " << beamspotXSize_ << ", " << beamspotYSize_ << "," << event->GetNumberOfPrimaryVertex() << std::endl;
            
        // Activate the plugin manager hook.        
        if (event->GetNumberOfPrimaryVertex() > 0){
            if (useBeamspot_) smearingBeamspot(event);
            pluginManager_->generatePrimary(event);
        }
    }

    void PrimaryGeneratorAction::smearingBeamspot(G4Event* event) {
        
        double IPWidthX = beamspotXSize_;
        double IPWidthY = beamspotYSize_;

        int nPV = event->GetNumberOfPrimaryVertex();
        for (int iPV = 0; iPV < nPV; ++iPV) {
            G4PrimaryVertex* curPV = event->GetPrimaryVertex(iPV);
            double x0_i = curPV->GetX0();
            double y0_i = curPV->GetY0();
            double z0_i = curPV->GetZ0();
            double x0_f = random_->Uniform( x0_i - IPWidthX, x0_i + IPWidthX );
            double y0_f = random_->Uniform( y0_i - IPWidthY, y0_i + IPWidthY );
            curPV->SetPosition( x0_f, y0_f, z0_i );
        }        

    }
}

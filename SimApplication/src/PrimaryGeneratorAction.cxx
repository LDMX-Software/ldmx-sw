#include "SimApplication/PrimaryGeneratorAction.h"
#include "SimApplication/UserPrimaryParticleInformation.h"
#include "SimApplication/PrimaryGeneratorMessenger.h"

// Geant4
#include "G4Event.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "G4SystemOfUnits.hh"

namespace ldmx {

    PrimaryGeneratorAction::PrimaryGeneratorAction() :
            G4VUserPrimaryGeneratorAction(), generator_(new G4ParticleGun), random_(new TRandom) {
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
        if (dynamic_cast<G4ParticleGun*>(generator_) !=  NULL) {

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

        if (PrimaryGeneratorMessenger::useBeamspot()) smearingBeamspot(event);
        
        // Activate the plugin manager hook.        
        pluginManager_->generatePrimary(event);

    }

    void PrimaryGeneratorAction::smearingBeamspot(G4Event* event) {
        
        std::cout << "[PrimaryGeneratorAction::smearingBeamspot] Number of vertices! " << event->GetNumberOfPrimaryVertex() << std::endl;
        double IPWidth = PrimaryGeneratorMessenger::getBeamspotSize();
        std::cout << "using beamspot? " << PrimaryGeneratorMessenger::useBeamspot() << ", size = " << IPWidth << std::endl;

        int nPV = event->GetNumberOfPrimaryVertex();
        for (int iPV = 0; iPV < nPV; ++iPV) {
            G4PrimaryVertex* curPV = event->GetPrimaryVertex(iPV);

            std::cout << "vertex, particle = " << iPV << ": " << curPV->GetX0() << "," << curPV->GetY0() << "," << curPV->GetZ0() << std::endl;
            double x0_i = curPV->GetX0();
            double y0_i = curPV->GetY0();
            double z0_i = curPV->GetZ0();
            double x0_f = random_->Uniform( x0_i - IPWidth, x0_i + IPWidth );
            double y0_f = random_->Uniform( y0_i - IPWidth, y0_i + IPWidth );
            curPV->SetPosition( x0_f, y0_f, z0_i );
            std::cout << "modified vertex, particle = " << iPV  << ": " << curPV->GetX0() << "," << curPV->GetY0() << "," << curPV->GetZ0() << std::endl;

        }        

    }
}

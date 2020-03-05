/**
 * @file PrimaryGeneratorAction.cxx
 * @brief Class implementing the Geant4 primary generator action
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "SimApplication/PrimaryGeneratorAction.h"

/*~~~~~~~~~~~~*/
/*   Geant4   */
/*~~~~~~~~~~~~*/
#include "G4Event.hh"
#include "G4RunManager.hh"  // Needed for CLHEP

/*~~~~~~~~~~~~~*/
/*   SimCore   */
/*~~~~~~~~~~~~~*/
#include "SimApplication/ParticleGun.h"
#include "SimApplication/UserPrimaryParticleInformation.h"

/*~~~~~~~~~~*/
/*   ROOT   */
/*~~~~~~~~~~*/
#include "TRandom3.h" 

namespace ldmx {

    PrimaryGeneratorAction::PrimaryGeneratorAction(Parameters& parameters) 
        : G4VUserPrimaryGeneratorAction() {

        // The parameters used to configure the primary generator action
        parameters_ = parameters;  

        // Instantiate the random number generator and set the seed.
        random_ = std::make_unique< TRandom3 >(); 
        random_->SetSeed( CLHEP::HepRandom::getTheSeed() );

        // Instantiate the manager 
        manager_ = std::make_unique< PrimaryGeneratorManager >(parameters); 
        
    }

    PrimaryGeneratorAction::~PrimaryGeneratorAction() {}

    /*
    void PrimaryGeneratorAction::setPrimaryGenerator(G4VPrimaryGenerator* aGenerator) {
      
        if ((dynamic_cast<MultiParticleGunPrimaryGenerator*>(aGenerator)) != NULL){
            indexMpg_ = ((int) generator_.size()) - 1;
        }
        if ((dynamic_cast<RootPrimaryGenerator*>(aGenerator)) != NULL){
            indexRpg_ = ((int) generator_.size()) - 1;
        }        
    }*/

    void PrimaryGeneratorAction::GeneratePrimaries(G4Event* event) {
      
        /// Get the list of generators that will be used for this event
        auto generators{manager_->getGenerators()}; 

        // Generator the primary vertex 
        std::for_each( generators.begin(), generators.end(), 
                [event](const auto& generator) { generator->GeneratePrimaryVertex(event); } );
        
        // Activate the plugin manager hook.  
        if (event->GetNumberOfPrimaryVertex() > 0) {
            if (useBeamspot_) smearingBeamspot(event);
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

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
#include "SimApplication/PrimaryGeneratorManager.h"
#include "SimApplication/UserPrimaryParticleInformation.h"
#include "Framework/FrameworkDef.h"

/*~~~~~~~~~~*/
/*   ROOT   */
/*~~~~~~~~~~*/
#include "TRandom3.h" 

namespace ldmx {

    PrimaryGeneratorAction::PrimaryGeneratorAction(Parameters& parameters) 
        : G4VUserPrimaryGeneratorAction(),
        manager_( PrimaryGeneratorManager::getInstance() )
    {

        // The parameters used to configure the primary generator action
        parameters_ = parameters;  

        // Instantiate the random number generator and set the seed.
        random_ = std::make_unique< TRandom3 >(); 
        random_->SetSeed( CLHEP::HepRandom::getTheSeed() );

        // Check whether a beamspot should be used or not.
        auto beamSpot{parameters.getParameter< std::vector< double > >("beamSpotSmear",{})};
        if (!beamSpot.empty()) {
            useBeamspot_ = true;
            beamspotXSize_ = beamSpot[0];
            beamspotYSize_ = beamSpot[1];
            beamspotZSize_ = beamSpot[2];
        }

        auto generators{parameters_.getParameter< std::vector< Class > >("generators")};
        if ( generators.size() == 0 ) {
            EXCEPTION_RAISE(
                    "MissingGenerator",
                    "Need to define some generator of primaries."
                    );
        }

        for ( auto& generator : generators ) {
            manager_.createGenerator(generator.className_, generator.instanceName_, generator.params_);
        }
        
    }

    PrimaryGeneratorAction::~PrimaryGeneratorAction() { }

    void PrimaryGeneratorAction::GeneratePrimaries(G4Event* event) {
      
        /// Get the list of generators that will be used for this event
        auto generators{manager_.getGenerators()}; 

        // Generator the primary vertex 
        std::for_each( generators.begin(), generators.end(), 
                [event](const auto& generator) { generator->GeneratePrimaryVertex(event); } );
        
        // smear all primary vertices (if activated)
        if (event->GetNumberOfPrimaryVertex() > 0) {
            setUserPrimaryInfo(event);
            if (useBeamspot_) smearingBeamspot(event);
        } else {
            EXCEPTION_RAISE(
                    "NoPrimaries",
                    "No primary vertices were produced by any of the generators."
                    );
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

    void PrimaryGeneratorAction::setUserPrimaryInfo(G4Event* event) {

        int nPV = event->GetNumberOfPrimaryVertex();
        for (int iPV = 0; iPV < nPV; ++iPV) {
            G4PrimaryVertex* vertex = event->GetPrimaryVertex(iPV);
                
            // Loop over all particle associated with the primary vertex and
            // set the generator status to 1.
            for (int iparticle = 0; iparticle < vertex->GetNumberOfParticle(); ++iparticle) { 
                G4PrimaryParticle* primary = vertex->GetPrimary(iparticle);

                auto primaryInfo{dynamic_cast<UserPrimaryParticleInformation*>(primary->GetUserInformation())};
                if( not primaryInfo ) {
                    // no user info defined
                    //  ==> make a new one
                    primaryInfo = new UserPrimaryParticleInformation;
                    primary->SetUserInformation( primaryInfo );
                }//check if primaryinfo is defined

                int hepStatus = primaryInfo->getHepEvtStatus();
                if ( hepStatus <= 0 ) {
                    //undefined hepStatus ==> set to 1
                    primaryInfo->setHepEvtStatus(1);
                }//check if hepStatus defined

            }//iparticle - loop over primary particles
        } //iPV - loop over primary vertices

    }
}

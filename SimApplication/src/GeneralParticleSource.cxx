/**
 * @file GeneralParticleSource.cxx
 * @brief Extension of G4GeneralParticleSource.
 * @author Tom Eichlersmith, University of Minnesota
 */

#include "SimApplication/GeneralParticleSource.h"

/*~~~~~~~~~~~~*/
/*   Geant4   */
/*~~~~~~~~~~~~*/
#include "G4Event.hh"
#include "G4UImanager.hh"

/*~~~~~~~~~~~~~~~*/
/*   Framework   */
/*~~~~~~~~~~~~~~~*/
#include "Framework/Parameters.h" 
#include "SimApplication/UserPrimaryParticleInformation.h"

namespace ldmx { 

    GeneralParticleSource::GeneralParticleSource(const std::string& name, Parameters& parameters) :
        PrimaryGenerator( name , parameters )
        {

            auto initCommands{parameters_.getParameter< std::vector<std::string> >( "initCommands" )};

            for ( const auto& cmd : initCommands ) {
                int g4Ret = G4UImanager::GetUIpointer()->ApplyCommand( cmd );
                if ( g4Ret > 0 ) {
                    EXCEPTION_RAISE(
                            "InitCmd",
                            "Initialization command '" + cmd + "' returned a failue status from Geant4: " + std::to_string(g4Ret)
                            );
                }
            }
    } 

    GeneralParticleSource::~GeneralParticleSource() {} 

    void GeneralParticleSource::GeneratePrimaryVertex(G4Event* event) { 
      
        //just pass to the Geant4 implementation
        theG4Source_.GeneratePrimaryVertex( event );

        // Set the generator status to 1 for particle gun primaries
        for (int ivertex = 0; ivertex < event->GetNumberOfPrimaryVertex(); ++ivertex) {
            
            // Get the ith primary vertex from the event
            auto vertex{event->GetPrimaryVertex(ivertex)}; 
            
            // Loop over all particle associated with the primary vertex and
            // set the generator status to 1.
            for (int iparticle = 0; iparticle < vertex->GetNumberOfParticle(); ++iparticle) { 
                auto primaryInfo{new UserPrimaryParticleInformation()};
                primaryInfo->setHepEvtStatus(1); 
                vertex->GetPrimary(iparticle)->SetUserInformation(primaryInfo);  
            }
        }

        return;
    }

} // ldmx

DECLARE_GENERATOR( ldmx , GeneralParticleSource )

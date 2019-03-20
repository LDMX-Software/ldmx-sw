/**
 * @file GeneralParticleSource.cxx
 * @brief Extension of G4GeneralParticleSource.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "SimApplication/GeneralParticleSource.h"

//------------//
//   Geant4   //
//------------//
#include "G4Event.hh"

//-------------//
//   ldmx-sw   //
//-------------//
#include "SimApplication/UserPrimaryParticleInformation.h"

namespace ldmx { 

    GeneralParticleSource::GeneralParticleSource() {} 

    GeneralParticleSource::~GeneralParticleSource() {} 

    void GeneralParticleSource::GeneratePrimaryVertex(G4Event* event) { 
        
        G4GeneralParticleSource::GeneratePrimaryVertex(event);

        // Set the generator status to 1 for particle gun primaries
        for (int ivertex = 0; ivertex < event->GetNumberOfPrimaryVertex(); ++ivertex) {
            
            // Get the ith primary vertex from the event
            G4PrimaryVertex* vertex = event->GetPrimaryVertex(ivertex);  
            
            // Loop over all particle associated with the primary vertex and
            // set the generator status to 1.
            for (int iparticle = 0; iparticle < vertex->GetNumberOfParticle(); ++iparticle) { 
                auto primaryInfo{new UserPrimaryParticleInformation()};
                primaryInfo->setHepEvtStatus(1); 
                vertex->GetPrimary(iparticle)->SetUserInformation(primaryInfo);  
            }
        }
    }
} // ldmx

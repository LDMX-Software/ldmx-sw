/**
 * @file ParticleGun.cxx
 * @brief Extension of G4ParticleGun.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 * @author Tom Eichlersmith, University of Minnesota
 */

#include "SimApplication/ParticleGun.h"

/*~~~~~~~~~~~~~~~~*/
/*   C++ StdLib   */
/*~~~~~~~~~~~~~~~~*/
#include <memory>

/*~~~~~~~~~~~~*/
/*   Geant4   */
/*~~~~~~~~~~~~*/
#include "G4Event.hh"
#include "G4ParticleTable.hh"
#include "G4SystemOfUnits.hh"
#include "G4ThreeVector.hh"

/*~~~~~~~~~~~~~*/
/*   SimCore   */
/*~~~~~~~~~~~~~*/
#include "SimApplication/UserPrimaryParticleInformation.h"

/*~~~~~~~~~~~~~~~*/
/*   Framework   */
/*~~~~~~~~~~~~~~~*/
#include "Framework/Parameters.h" 

namespace ldmx { 

    ParticleGun::ParticleGun(const std::string& name, Parameters& parameters) :
        PrimaryGenerator( name , parameters )
        {

        verbosity_ = parameters.getParameter< int >("verbosity");

        auto particleTable{G4ParticleTable::GetParticleTable()};
        
        auto particle{parameters.getParameter< std::string >("particle")};
        if (auto particleDef{particleTable->FindParticle(particle)}; particleDef != 0) {
            if ( verbosity_ > 1 ) {
                std::cout << "[ ParticleGun ] : Firing particle of type " << particle << std::endl; 
            }
            theGun_.SetParticleDefinition(particleDef); 
        }

        auto energy{parameters.getParameter< double >("energy")};
        if ( verbosity_ > 1 ) {
            std::cout << "[ ParticleGun ] : Setting energy to " << energy*GeV << std::endl;
        }
        theGun_.SetParticleEnergy(energy*GeV); 

        auto position{parameters.getParameter< std::vector<double> >("position")};
        if (!position.empty()) {
            G4ThreeVector pVec(position[0]*mm, position[1]*mm, position[2]*mm); 
            if ( verbosity_ > 1 ) {
                std::cout << "[ ParticleGun ] : position " << pVec << std::endl;
            }
            theGun_.SetParticlePosition(pVec);
        }

        auto time{parameters.getParameter< double >("time")}; 
        if (time < 0) time = 0.0; 
        if ( verbosity_ > 1 ) {
            std::cout << "[ ParticleGun ] : Setting particle time  to " << time << std::endl;
        }
        theGun_.SetParticleTime(time*ns); 

        auto direction{parameters.getParameter< std::vector<double > >("direction")};
        if (!direction.empty()) { 
            G4ThreeVector dVec(direction[0], direction[1], direction[2]); 
            if ( verbosity_ > 1 ) {
                std::cout << "[ ParticleGun ] : direction " << dVec.unit() << std::endl;
            }
            theGun_.SetParticleMomentumDirection(dVec); 
        }
    } 

    ParticleGun::~ParticleGun() {} 

    void ParticleGun::GeneratePrimaryVertex(G4Event* event) { 
      
        // Call G4 class method to generate primaries. 
        theGun_.GeneratePrimaryVertex(event);

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
    }

} // ldmx

DECLARE_GENERATOR( ldmx , ParticleGun )

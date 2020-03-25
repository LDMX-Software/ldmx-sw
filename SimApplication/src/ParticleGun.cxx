/**
 * @file ParticleGun.cxx
 * @brief Extension of G4ParticleGun.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
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

    ParticleGun::ParticleGun(Parameters& parameters) {

        auto particleTable{G4ParticleTable::GetParticleTable()};
        
        auto particle{parameters.getParameter< std::string >("gun.particle")};
        if (auto particleDef{particleTable->FindParticle(particle)}; particleDef != 0) {
            //std::cout << "ParticleGun::ParticleGun : Firing particle of type " << particle << std::endl; 
            SetParticleDefinition(particleDef); 
        }

        auto energy{parameters.getParameter< double >("gun.energy")};
        //std::cout << "ParticleGun::ParticleGun : Setting energy to " << energy*GeV << std::endl;
        SetParticleEnergy(energy*GeV); 

        auto position{parameters.getParameter< std::vector<double> >("gun.position")};
        if (!position.empty()) {
            G4ThreeVector pVec(position[0]*mm, position[1]*mm, position[2]*mm); 
            //std::cout << "ParticleGun::ParticleGun : position " << pVec << std::endl;
            SetParticlePosition(pVec);
        }

        auto time{parameters.getParameter< double >("gun.time")}; 
        if (time < 0) time = 0.0; 
        //std::cout << "ParticleGun::ParticleGun : Setting particle time  to " << time << std::endl;
        SetParticleTime(time*ns); 

        auto direction{parameters.getParameter< std::vector<double > >("gun.direction")};
        if (!direction.empty()) { 
            G4ThreeVector dVec(direction[0], direction[1], direction[2]); 
            //std::cout << "ParticleGun::ParticleGun : direction " << dVec.unit() << std::endl;
            SetParticleMomentumDirection(dVec); 
        }
    } 

    ParticleGun::~ParticleGun() {} 

    void ParticleGun::GeneratePrimaryVertex(G4Event* event) { 
      
        // Call base class method to generate primaries. 
        G4ParticleGun::GeneratePrimaryVertex(event);

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

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

/*~~~~~~~~~~~~~~~*/
/*   Framework   */
/*~~~~~~~~~~~~~~~*/
#include "Framework/Parameters.h" 

namespace ldmx { 

    ParticleGun::ParticleGun(const std::string& name, Parameters& parameters) :
        PrimaryGenerator( name , parameters )
        {

        auto particleTable{G4ParticleTable::GetParticleTable()};
        
        auto particle{parameters.getParameter< std::string >("particle")};
        if (auto particleDef{particleTable->FindParticle(particle)}; particleDef != 0) {
            ldmx_log(info) << "Firing particle of type " << particle;
            theGun_.SetParticleDefinition(particleDef); 
        }

        auto energy{parameters.getParameter< double >("energy")};
        ldmx_log(info) << "Setting energy to " << energy*GeV;
        theGun_.SetParticleEnergy(energy*GeV); 

        auto position{parameters.getParameter< std::vector<double> >("position")};
        if (!position.empty()) {
            G4ThreeVector pVec(position[0]*mm, position[1]*mm, position[2]*mm); 
            ldmx_log(info) << "position " << pVec;
            theGun_.SetParticlePosition(pVec);
        }

        auto time{parameters.getParameter< double >("time")}; 
        if (time < 0) time = 0.0; 
        ldmx_log(info) << "Setting particle time  to " << time;
        theGun_.SetParticleTime(time*ns); 

        auto direction{parameters.getParameter< std::vector<double > >("direction")};
        if (!direction.empty()) { 
            G4ThreeVector dVec(direction[0], direction[1], direction[2]); 
            ldmx_log(info) << "direction " << dVec.unit();
            theGun_.SetParticleMomentumDirection(dVec); 
        }
    } 

    ParticleGun::~ParticleGun() {} 

    void ParticleGun::GeneratePrimaryVertex(G4Event* event) { 
      
        // Call G4 class method to generate primaries. 
        theGun_.GeneratePrimaryVertex(event);

    }

} // ldmx

DECLARE_GENERATOR( ldmx , ParticleGun )

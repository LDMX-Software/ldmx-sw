/**
 * @file ParticleGun.h
 * @brief Extension of G4ParticleGun.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef _SIM_APPLICATION_PARTICLE_GUN_H_
#define _SIM_APPLICATION_PARTICLE_GUN_H_

//------------//
//   Geant4   //
//------------//
#include "G4ParticleGun.hh"

// Forward declarations
class G4Event; 

namespace ldmx { 

    class ParticleGun : public G4ParticleGun { 
    
        public: 

            /** Constructor. */
            ParticleGun(); 

            /** Destructor. */
            ~ParticleGun();

            /** 
             * Generate the primary vertices in the Geant4 event. 
             * @param event The Geant4 event.
             */
            void GeneratePrimaryVertex(G4Event* event); 

    }; // ParticleGun 

} // ldmx

#endif // _SIM_APPLICATION_PARTICLE_GUN_H_

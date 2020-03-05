/**
 * @file ParticleGun.h
 * @brief Extension of G4ParticleGun.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef SIMCORE_PARTICLE_GUN_H
#define SIMCORE_PARTICLE_GUN_H

//------------//
//   Geant4   //
//------------//
#include "G4ParticleGun.hh"

// Forward declarations
class G4Event; 

namespace ldmx {

    class Parameters;  

    class ParticleGun : public G4ParticleGun { 
    
        public: 

            /** 
             * Constructor. 
             *
             * @param parameters Parameters used to configure the particle gun. 
             */
            ParticleGun(Parameters& parameters); 

            /// Destructor
            ~ParticleGun();

            /** 
             * Generate the primary vertices in the Geant4 event. 
             * @param event The Geant4 event.
             */
            void GeneratePrimaryVertex(G4Event* event); 

    }; // ParticleGun 

} // ldmx

#endif // SIMCORE_PARTICLE_GUN_H

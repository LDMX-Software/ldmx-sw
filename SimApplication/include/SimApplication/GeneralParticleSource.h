/**
 * @file GeneralParticleSource.h
 * @brief Extension of G4GeneralParticleSource.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef _SIM_APPLICATION_GENERAL_PARTICLE_SOURCE_H_
#define _SIM_APPLICATION_GENERAL_PARTICLE_SOURCE_H_

//------------//
//   Geant4   //
//------------//
#include "G4GeneralParticleSource.hh"

// Forward declarations
class G4Event; 

namespace ldmx { 

    class GeneralParticleSource : public G4GeneralParticleSource { 
    
        public: 

            /** Constructor. */
            GeneralParticleSource(); 

            /** Destructor. */
            ~GeneralParticleSource();

            /** 
             * Generate the primary vertices in the Geant4 event. 
             * @param event The Geant4 event.
             */
            void GeneratePrimaryVertex(G4Event* event); 

    }; // GeneralParticleSource 

} // ldmx

#endif // _SIM_APPLICATION_GENERAL_PARTICLE_SOURCE_H_

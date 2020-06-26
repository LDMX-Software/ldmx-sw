/**
 * @file ParticleGun.h
 * @brief Extension of G4ParticleGun.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 * @author Tom Eichlersmith, University of Minnesota
 */

#ifndef SIMAPPLICATION_PARTICLE_GUN_H
#define SIMAPPLICATION_PARTICLE_GUN_H

//------------//
//   Geant4   //
//------------//
#include "G4ParticleGun.hh"

//------------//
//   LDMX     //
//------------//
#include "SimApplication/PrimaryGenerator.h"

// Forward declarations
class G4Event; 

namespace ldmx {

    // Forward declarations
    class Parameters;  

    /**
     * @class ParticleGun
     * @brief Class that extends the functionality of G4ParticleGun.
     */
    class ParticleGun : public PrimaryGenerator { 
    
        public: 

            /** 
             * Constructor. 
             *
             * @param parameters Parameters used to configure the particle gun. 
             *
             * Parameters:
             *  verbosity: > 1 means print configuration
             *  particle : name of particle to shoot (Geant4 naming)
             *  energy   : energy of particle (GeV)
             *  position : position to shoot from (mm three-vector)
             *  time     : time to shoot at (ns)
             *  direction: direction to shoot in (unitless three-vector)
             */
            ParticleGun(const std::string& name, Parameters& parameters); 

            /// Destructor
            ~ParticleGun();

            /** 
             * Generate the primary vertices in the Geant4 event. 
             * 
             * @param event The Geant4 event.
             */
            void GeneratePrimaryVertex(G4Event* event) final override; 

        private:

            /**
             * The actual Geant4 implementation of the ParticleGun
             */
            G4ParticleGun theGun_;

            /**
             * LDMX Verbosity for this generator
             */
            int verbosity_;

    }; // ParticleGun 

} // ldmx

#endif // SIMAPPLICATION_PARTICLE_GUN_H

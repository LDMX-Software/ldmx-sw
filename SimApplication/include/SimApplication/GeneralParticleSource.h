/**
 * @file GeneralParticleSource.h
 * @brief Extension of G4GeneralParticleSource.
 * @author Tom Eichlersmith, University of Minnesota
 */

#ifndef SIMAPPLICATION_GENERALPARTICLESOURCE_H
#define SIMAPPLICATION_GENERALPARTICLESOURCE_H

//------------//
//   Geant4   //
//------------//
#include "G4GeneralParticleSource.hh"

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
     * @class GeneralParticleSource
     * @brief Class that extends the functionality of G4GeneralParticleSource.
     */
    class GeneralParticleSource : public PrimaryGenerator { 
    
        public: 

            /** 
             * Constructor. 
             *
             * @param parameters Parameters used to configure the particle gun. 
             *
             * Parameters:
             *  initCommands : vector of Geant4 strings to initialize the GPS
             */
            GeneralParticleSource(const std::string& name, Parameters& parameters); 

            /// Destructor
            ~GeneralParticleSource();

            /** 
             * Generate the primary vertices in the Geant4 event. 
             * 
             * @param event The Geant4 event.
             */
            void GeneratePrimaryVertex(G4Event* event) final override; 

        private:

            /**
             * The underlying Geant4 GPS implementation.
             *
             * The creation of this class creates a new messenger that we can pass commands to.
             */
            G4GeneralParticleSource theG4Source_;

    }; // GeneralParticleSource 

} // ldmx

#endif // SIMAPPLICATION_GENERALPARTICLESOURCE_H

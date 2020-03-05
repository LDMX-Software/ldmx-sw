/**
 * @file PrimaryGeneratorAction.h
 * @brief Class implementing the Geant4 primary generator action
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef SIMAPPLICATION_PRIMARYGENERATORACTION_H
#define SIMAPPLICATION_PRIMARYGENERATORACTION_H

/*~~~~~~~~~~~~~~~~*/
/*   C++ StdLib   */
/*~~~~~~~~~~~~~~~~*/
#include <algorithm>
#include <memory>

/*~~~~~~~~~~~~*/
/*   Geant4   */
/*~~~~~~~~~~~~*/
#include "G4VUserPrimaryGeneratorAction.hh"

/*~~~~~~~~~~~~~*/
/*   SimCore   */
/*~~~~~~~~~~~~~*/
//#include "SimApplication/MultiParticleGunPrimaryGenerator.h"
#include "SimApplication/PrimaryGeneratorManager.h" 
//#include "SimApplication/RootPrimaryGenerator.h"

/*~~~~~~~~~~~~~~~*/
/*   Framework   */
/*~~~~~~~~~~~~~~~*/
#include "Framework/Parameters.h"

// Forward declarations
class G4Event; 
class TRandom3;

namespace ldmx {

    class ParticleGun; 

    /**
     * @class PrimaryGeneratorAction
     * @brief Implementation of Geant4 primary generator action
     */
    class PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction {

        public:

            /*
             * Constructor
             *
             * @param parameters The parameters used to configure the primary
             *                   generator action. 
             */  
            PrimaryGeneratorAction(Parameters& parameters);

            /**
             * Class destructor.
             */
            virtual ~PrimaryGeneratorAction();

            /**
             * Generate the event.
             * @param event The Geant4 event.
             */
            void GeneratePrimaries(G4Event* event) final override;

            /**
             * Enable beamspot smearing.
             * @param bool
             */
            void setUseBeamspot(bool usebs) { useBeamspot_ = usebs; };

            /**
             * Set beamspot size in x.
             * @param beamspot size
             */
            void setBeamspotXSize(double bssize){ beamspotXSize_ = bssize; };
            
            /**
             * Set beamspot size in y.
             * @param beamspot size
             */
            void setBeamspotYSize(double bssize){ beamspotYSize_ = bssize; };

            /**
             * Set beamspot size in z.
             * @param beamspot size
             */
            void setBeamspotZSize(double bssize){ beamspotZSize_ = bssize; };

            /** 
             * Get the index of the last generator in the list of 
             * generators.
             */
            int getIndexMPG(){ return indexMpg_; }
            int getIndexRPG(){ return indexRpg_; }

        private:

            /**
             * Smearing beamspot
             * @param event The Geant4 event.
             */
            void smearingBeamspot(G4Event* event);

            /// Manager of all generators used by the event
            std::unique_ptr< PrimaryGeneratorManager > manager_;

            /// Random number generator
            std::unique_ptr< TRandom3 > random_;

            /// The parameters used to configure the primary generator of choice
            Parameters parameters_; 

            /** 
             * Flag denoting whether the vertex position of a particle 
             * should be smeared.
             */ 
            bool useBeamspot_{false};
            
            /** Extent of the beamspot in x. */
            double beamspotXSize_{0};            
            
            /** Extent of the beamspot in y. */
            double beamspotYSize_{0};   

            /** Extent of the beamspot in y. */
            double beamspotZSize_{0.};   

            /** The index of the last generator in the list of generators. */ 
            int indexMpg_{-1};          
            int indexRpg_{-1};          

    };  // PrimaryGeneratorAction

} // ldmx

#endif // SIMAPPLICATION_PRIMARYGENERATORACTION_H

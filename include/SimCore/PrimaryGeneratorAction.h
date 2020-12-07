/**
 * @file PrimaryGeneratorAction.h
 * @brief Class implementing the Geant4 primary generator action
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef SIMCORE_PRIMARYGENERATORACTION_H
#define SIMCORE_PRIMARYGENERATORACTION_H

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
#include "SimCore/PluginFactory.h"

/*~~~~~~~~~~~~~~~*/
/*   Framework   */
/*~~~~~~~~~~~~~~~*/
#include "Framework/Configure/Parameters.h"

// Forward declarations
class G4Event; 
class TRandom3;

namespace ldmx {

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

        private:

            /**
             * Smearing beamspot
             * @param event The Geant4 event.
             */
            void smearingBeamspot(G4Event* event);

            /**
             * Set UserInformation for primary vertices if they haven't been set before.
             *
             * Some features downstream of the primaries require certain user info to function
             * properly. This ensures that it happens.
             *
             * Makes sure that each particle on each primary vertex has
             *  1. A defined UserPrimaryParticleInformation member
             *  2. The HepEvtStatus for this primary info is non-zero
             *
             * @param event Geant4 event to go through primaries
             */
            void setUserPrimaryInfo(G4Event* event);

            /// Manager of all generators used by the event
            simcore::PluginFactory &manager_;

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

            /**
             * Should we time-shift so that the primary vertices arrive (or originate)
             * at t=0ns at z=0mm?
             *
             * @note This should remain true unless the user knows what they are doing!
             */
            bool time_shift_primaries_{true};

    };  // PrimaryGeneratorAction

} // ldmx

#endif // SIMCORE_PRIMARYGENERATORACTION_H

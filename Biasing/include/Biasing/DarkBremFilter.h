/**
 * @file DarkBremFilter.h
 * @class DarkBremFilter
 * @brief Class defining a UserActionPlugin that allows a user to filter out 
 *        events that don't result in a dark brem inside a given volume
 * @author Michael Revering, University of Minnesota
 * @author Tom Eichlersmith, University of Minnesota
 */

#ifndef BIASING_DARKBREMFILTER_H_
#define BIASING_DARKBREMFILTER_H_

//----------------//
//   C++ StdLib   //
//----------------//
#include <algorithm>

//------------//
//   Geant4   //
//------------//
#include "G4RunManager.hh"

/*~~~~~~~~~~~~~*/
/*   SimCore   */
/*~~~~~~~~~~~~~*/
#include "SimCore/UserAction.h"

/*~~~~~~~~~~~~~~~*/
/*   Framework   */
/*~~~~~~~~~~~~~~~*/
#include "Framework/Parameters.h" 

namespace ldmx {

    /**
     * @class DarkBremFilter
     *
     * This class is meant to filter for events that produce a dark brem matching
     * the input parameters:
     *      volume: A' originates inside of the input volume (target or ecal)
     *      nGensFromPrimary: A' produced by electron removed from primary by <= input n generations
     *
     * The general idea for this filter is to force Geant4 to
     * simulate the tracks in generation order. This is done by
     * pushing all new tracks to the waiting stack and incrementing
     * the current generation by one when the urgent stack is empty.
     * Checking if the A' is produced is done in the PostTrackingAction,
     * and the event is aborted if we reach a generation later than the input
     * generation and no A' has been found.
     */
    class DarkBremFilter : public UserAction {

        public:

            /**
             * Class constructor.
             *
             * Links this filter to its messenger.
             */
            DarkBremFilter(const std::string& name, Parameters& parameters);

            /**
             * Class destructor.
             */
            ~DarkBremFilter() { }

            /**
             * Get the types of actions this class can do
             */
            std::vector< TYPE > getTypes() final override {
                return { TYPE::STACKING , TYPE::EVENT , TYPE::TRACKING };
            }

            /**
             * Reset generation counter and flag on if A' has been found
             */
            void BeginOfEventAction(const G4Event* ) final override;

            /**
             * Classify a new track which postpones track processing.
             *
             * Push all new tracks to fWaiting
             *
             * This forces all tracks in a given generation to be simulated
             * before the next generation of tracks is started.
             *
             * Checks a new track for being an A'
             *  if it is an A', checks if it is in correct volume and sets foundAp_ member
             * 
             * @param aTrack The Geant4 track.
             * @param currentTrackClass The current track classification.
             */
            G4ClassificationOfNewTrack ClassifyNewTrack(const G4Track* aTrack, 
                    const G4ClassificationOfNewTrack& currentTrackClass) final override; 

            /**
             * Increment the generation counter
             *
             * This function is called when the urgent stack is empty
             * and the waiting stack is transferred to the urgent stack.
             *
             * With all new tracks being pushed to the waiting stack,
             * this only occurs when a new generation has begun.
             *
             * When a new generation has begun, if the new generation is more
             * than the input generation limit, we check if the A' was found.
             */
            void NewStage() final override;

            /**
             * Make sure A' is saved.
             *
             * If passed track is A', set save status to true
             * Aborts event if A' does not originate in desired volume
             */
            void PostUserTrackingAction(const G4Track* ) final override;

        private:

            /**
             * Check if input volume is in the desired volume name
             *
             * @param pointer to track to check
             * @return true if originated in desired volume
             */
            bool inDesiredVolume(const G4Track* ) const;

        private:

            /** 
             * The volumes that the filter will be applied to.
             *
             * Default: 'target'
             *
             * Parameter Name: 'volume'
             *  Searched for in PhysicalVolumeStore
             */
            std::vector< G4LogicalVolume* > volumes_;

            /**
             * Number of generations away from primary
             * to allow to dark brem.
             *
             * Default: 0 (only primary itself)
             *
             * Parameter Name: 'nGensFromPrimary'
             */
            int nGensFromPrimary_;

            /**
             * Minimum energy [MeV] that the A' should have to keep the event.
             * 
             * Default: 0 (allow all energies)
             *
             * Parameter Name: 'minApEnergy'
             */
            double minApEnergy_;

            /**
             * Have we found the A' yet?
             */
            bool foundAp_;

            /**
             * The current generation removed from the primary electron
             */
            int currentGen_;

    }; // DarkBremFilter
}

#endif // BIASING_DARKBREMFILTER_H__

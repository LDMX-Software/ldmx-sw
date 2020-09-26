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
     *
     *      volume: A' originates inside of the input volume (target or ecal)
     *      nGensFromPrimary: A' produced by electron removed from primary by <= input n generations
     *      minApEnergy: minimum energy [MeV] A' needs to have
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
             * Retrieve the necessary configuration parameters
             */
            DarkBremFilter(const std::string& name, Parameters& parameters);

            /**
             * Class destructor.
             */
            ~DarkBremFilter() { }

            /**
             * Get the types of actions this class can do
             *
             * @return list of action types this class does
             */
            std::vector< TYPE > getTypes() final override {
                return { TYPE::STACKING , TYPE::EVENT , TYPE::TRACKING };
            }

            /**
             * Reset generation counter and flag on if A' has been found
             *
             * @param event unused
             */
            void BeginOfEventAction(const G4Event* event) final override;

            /**
             * Classify a new track which postpones track processing.
             *
             * Push all new tracks to fWaiting
             *
             * This forces all tracks in a given generation to be simulated
             * before the next generation of tracks is started.
             *
             * Checks a new track for being an A'
             *  if it is an A', sets the foundAp_ member and returns fUrgent
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
             *  or has the minimum energy we wish
             *
             * @param track G4Track to check if it is an A'
             */
            void PostUserTrackingAction(const G4Track* track) final override;

        private:

            /**
             * Check if input volume is in the desired volume name
             *
             * @param pointer to track to check
             * @return true if originated in desired volume
             */
            bool inDesiredVolume(const G4Track* ) const;

            /**
             * Helper to abort an event with a message
             *
             * Tells the RunManger to abort the current event
             * after displaying the input message.
             *
             * @param[in] reason reason for aborting the event
             */
            void AbortEvent(const std::string& reason) const;

        private:

            /** 
             * The volumes that the filter will be applied to.
             *
             * Parameter Name: 'volume'
             *  Searched for in PhysicalVolumeStore
             */
            std::vector< G4LogicalVolume* > volumes_;

            /**
             * Number of generations away from primary
             * to allow to dark brem.
             *
             * Parameter Name: 'nGensFromPrimary'
             */
            int nGensFromPrimary_;

            /**
             * Minimum energy [MeV] that the A' should have to keep the event.
             * 
             * Parameter Name: 'minApEnergy'
             */
            double minApEnergy_;

            /**
             * Have we found the A' yet?
             *
             * Reset to false in BeginOfEventAction
             */
            bool foundAp_;

            /**
             * The current generation removed from the primary electron
             *
             * Reset to zero in BeginOfEventAction
             */
            int currentGen_;

    }; // DarkBremFilter
}

#endif // BIASING_DARKBREMFILTER_H__

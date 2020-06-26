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
/*   SimApplication   */
/*~~~~~~~~~~~~~*/
#include "SimApplication/UserAction.h"

/*~~~~~~~~~~~~~~~*/
/*   Framework   */
/*~~~~~~~~~~~~~~~*/
#include "Framework/Parameters.h" 

namespace ldmx {

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
            ~DarkBremFilter();

            /**
             * Get the types of actions this class can do
             */
            std::vector< TYPE > getTypes() final override {
                return { TYPE::STACKING , TYPE::STEPPING };
            }

            /**
             * Kills events that don't contain a DarkBrem produced by the primary electron within the desired volume.
             *
             * @param step The Geant4 step.
             */
            void stepping(const G4Step* step);

            /**
             * Classify a new track which postpones track processing.
             * Track processing resumes normally if an interaction occurred.
             * @param aTrack The Geant4 track.
             * @param currentTrackClass The current track classification.
             */
            G4ClassificationOfNewTrack ClassifyNewTrack(const G4Track* aTrack, 
                    const G4ClassificationOfNewTrack& currentTrackClass) final override;

        private:

            /**
             * Checks if the secondaries given has an A Prime.
             *
             * @param secondaries list to search
             * @return true if A Prime in secondaries
             */
            bool hasAPrime(const G4TrackVector *secondaries) const;

        private:
            
            /** 
             * Level of verbosity for this filter
             *
             * Parameter Name: 'verbosity'
             */
            int verbosity_;

            /** 
             * The volume that the filter will be applied to.
             *
             * Parameter Name: 'volume'
             */
            std::string volumeName_;

    }; // DarkBremFilter
}

#endif // BIASING_DARKBREMFILTER_H__

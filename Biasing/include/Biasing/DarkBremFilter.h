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

//----------//
//   LDMX   //
//----------//
#include "SimPlugins/UserActionPlugin.h"
#include "Biasing/BiasingMessenger.h"
#include "Biasing/DarkBremFilterMessenger.h"

namespace ldmx {

    class DarkBremFilterMessenger; 

    class DarkBremFilter : public UserActionPlugin {

        public:

            /**
             * Class constructor.
             *
             * Links this filter to its messenger.
             */
            DarkBremFilter();

            /**
             * Class destructor.
             */
            ~DarkBremFilter();

            /**
             * Get the name of the plugin.
             * @return The name of the plugin.
             */
            virtual std::string getName() {
                return "DarkBremFilter";
            }

            /**
             * Not implemented right now, but want to investigate using it
             * Get whether this plugin implements the event action.
             *
             * @return True if the plugin implements the event action.
             */
//            virtual bool hasEventAction() { 
//                return true;
//            }

            /**
             * Get whether this plugin implements the stepping action.
             * @return True to indicate this plugin implements the stepping action.
             */
            bool hasSteppingAction() {
                return true;
            }

            /**
             * Get whether this plugin implements the stacking aciton.
             * @return True to indicate this plugin implements the stacking action.
             */
            bool hasStackingAction() { 
                return true;
            }

            /**
             * Kills events that don't contain a DarkBrem produced by the primary electron within the desired volume.
             *
             * @param step The Geant4 step.
             */
            void stepping(const G4Step* step);

            /**
             * Not implemented right now, but want to investigate using it
             * End of event action.
             */
            //virtual void endEvent(const G4Event*);

            /**
             * Classify a new track which postpones track processing.
             * Track processing resumes normally if an interaction occurred.
             * @param aTrack The Geant4 track.
             * @param currentTrackClass The current track classification.
             */
            G4ClassificationOfNewTrack stackingClassifyNewTrack(const G4Track* aTrack, 
                    const G4ClassificationOfNewTrack& currentTrackClass);

            /** 
             * @param volume Set the volume that the filter will be applied to. 
             */
            void setVolume(std::string volumeName) { volumeName_ = volumeName; }; 

        private:

            /**
             * Checks if the secondaries given has an A Prime.
             *
             * @param secondaries list to search
             * @return true if A Prime in secondaries
             */
            bool hasAPrime(const G4TrackVector *secondaries) const;

        private:
            
            /** Messenger used to pass arguments to this class. */
            DarkBremFilterMessenger* messenger_{nullptr};

            /** The volume that the filter will be applied to. */
            G4String volumeName_{"target_PV"};

    }; // DarkBremFilter
}

#endif // BIASING_DARKBREMFILTER_H__

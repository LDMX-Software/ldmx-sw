/**
 * @file UserTrackingAction.h
 * @brief Class which implements the user tracking action
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef SIMCORE_USERTRACKINGACTION_H_
#define SIMCORE_USERTRACKINGACTION_H_

/*~~~~~~~~~~~~~~~~*/
/*   C++ StdLib   */
/*~~~~~~~~~~~~~~~~*/
#include <vector>

// LDMX
#include "SimCore/TrackMap.h"
#include "SimCore/Trajectory.h"

// Geant4
#include "G4RunManager.hh"
#include "G4UserTrackingAction.hh"

/*~~~~~~~~~~~~~*/
/*   SimCore   */
/*~~~~~~~~~~~~~*/
#include "SimCore/UserAction.h" 

namespace ldmx {

    /**
     * @class UserTrackingAction
     * @brief Implementation of user tracking action
     *
     * Here, we manage the interaction between our track storage machinery (TrackMap)
     * and Geant4's tracking manager (G4TrackingManager).
     */
    class UserTrackingAction : public G4UserTrackingAction {

        public:

            /**
             * Class constructor.
             */
            UserTrackingAction() {}

            /**
             * Class destructor.
             */
            virtual ~UserTrackingAction() {}

            /**
             * Implementation of pre-tracking action.
             *
             * This is called whenever a track is going to start
             * being processed.
             *
             * @note A track could go through this function more than
             * once in an event if it is suspended.
             *
             * We first check if we have seen this track before by looking
             * inside of our track map.
             *
             * If we have seen it before, then we simply make sure
             * that our track map and Geant4's tracking manager are
             * on the same page about whether or not to store this track.
             *
             * If we haven't seen it before, then we must do some setup.
             * - Make an instance of UserTrackInformation and attach it to the track
             *   after setting the initial momentum and vertex volume.
             * - Using the gen status of the track and the region the track
             *   was produced in, decide if we will store the track by default.
             *   (Other user tracking actions can change the save flag in the track information.)
             *
             * We choose to store the track by default if it is a primary
             * or if it was created within a region where the 'StoreSecondaries' flag
             * was set to true.
             *
             * No matter what, if the track has parents, we insert the track
             * into the track map's ancestry tree.
             *
             * Finally, before we wrap up, we call any other tracking actions'
             * 'PreUserTrackingAction' methods.
             *
             * @param aTrack The Geant4 track.
             */
            void PreUserTrackingAction(const G4Track* aTrack);

            /**
             * Implementation of post-tracking action.
             *
             * We start by calling any other tracking actions'
             * PostUserTrackingAction methods.
             *
             * If the track's user information has a save flag
             * set to true, then we make sure it will be stored by
             * checking our track map and calling storeTrajectory
             * if it isn't within the track map.
             *
             * Finally, if the Geant4 tracking manager agrees that
             * this track will be stored and the trajectory is
             * going to be killed, then we retrieve the end point
             * momentum and pass that onto the trajectory.
             *
             * @param aTrack The Geant4 track.
             */
            void PostUserTrackingAction(const G4Track* aTrack);

            /**
             * Get a pointer to the current TrackMap for the event.
             * @return A pointer to the current TrackMap for the event.
             */
            TrackMap* getTrackMap() {
                return &trackMap_;
            }

            /**
             * Store a Trajectory for the given G4Track.
             *
             * We make sure the Geant4 tracking manager will
             * also store this track and then we create a 
             * Trajectory object for the track's storage.
             *
             * If the track is a primary particle, we pass
             * the HepEvtStatus as the trajectory's gen status.
             *
             * Finally, we add the newly created trajectory
             * to the track map.
             *
             * @param aTrack The Geant4 track.
             */
            void storeTrajectory(const G4Track* aTrack);

            /**
             * Get a pointer to the current UserTrackingAction from the G4RunManager.
             * @return A pointer to the current UserTrackingAction.
             */
            static UserTrackingAction* getUserTrackingAction() {
                return static_cast<UserTrackingAction*>(const_cast<G4UserTrackingAction*>(G4RunManager::GetRunManager()->GetUserTrackingAction()));
            }

            /**
             * Register a user action of type RunAction with this class. 
             *
             * @param action  User action of type RunAction
             */
            void registerAction(UserAction* trackingAction) { trackingActions_.push_back(trackingAction); }

        private:

            /// custom user actions to be called before and after processing a track
            std::vector<UserAction*> trackingActions_; 

            /** Stores parentage information for all tracks in the event. */
            TrackMap trackMap_;
    }; // UserTrackingAction
} // namespace ldmx

#endif

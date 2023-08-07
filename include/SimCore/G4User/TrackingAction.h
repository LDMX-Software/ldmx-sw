/**
 * @file TrackingAction.h
 * @brief Class which implements the user tracking action
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef SIMCORE_G4USER_USERTRACKINGACTION_H_
#define SIMCORE_G4USER_USERTRACKINGACTION_H_

/*~~~~~~~~~~~~~~~~*/
/*   C++ StdLib   */
/*~~~~~~~~~~~~~~~~*/
#include <vector>

// LDMX
#include "SimCore/TrackMap.h"

// Geant4
#include "G4RunManager.hh"
#include "G4UserTrackingAction.hh"

/*~~~~~~~~~~~~~*/
/*   SimCore   */
/*~~~~~~~~~~~~~*/
#include "SimCore/UserAction.h"

namespace simcore::g4user {

/**
 * @class TrackingAction
 * @brief Implementation of user tracking action
 *
 * Here, we manage the interaction between our track storage machinery
 * (TrackMap) and Geant4's tracking manager (G4TrackingManager).
 */
class TrackingAction : public G4UserTrackingAction {
 public:
  /**
   * Class constructor.
   */
  TrackingAction() {}

  /**
   * Class destructor.
   */
  virtual ~TrackingAction() {}

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
   * If we have seen it before, then we simply give
   * the track to our PreUserTrackingActions.
   *
   * If we haven't seen it before, then we must do some setup.
   * - Make an instance of UserTrackInformation and attach it to the track
   *   after setting the initial momentum and vertex volume.
   * - Using the gen status of the track and the region the track
   *   was produced in, decide if we will store the track by default.
   *   (Other user tracking actions can change the save flag in the track
   *   information.)
   *
   * We choose to store the track by default if any of the following are true
   * about the track
   * - it is a primary (gen status is one)
   * - it was created in a region without region info
   * - it was created in a region where the 'StoreSecondaries' flag
   *   was set to true.
   *
   * No matter what, we insert the track into the track map for book-keeping.
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
   * If the track should be saved (it's save flag is set to true)
   * and it is being stopped, then we save it in the track map.
   *
   * @note This is where we make the final decision on if a
   * particle should be saved into the output file.
   *
   * @see TrackMap::save for how a G4Track is translated
   * into our output SimParticle object.
   *
   * @param aTrack The Geant4 track.
   */
  void PostUserTrackingAction(const G4Track* aTrack);

  /**
   * Get a handle to the current TrackMap for the event.
   * @return A pointer to the current TrackMap for the event.
   */
  TrackMap& getTrackMap() { return trackMap_; }

  /**
   * Get a pointer to the current UserTrackingAction from the G4RunManager.
   * @return A pointer to the current UserTrackingAction.
   */
  static TrackingAction* get() {
    return static_cast<TrackingAction*>(const_cast<G4UserTrackingAction*>(
        G4RunManager::GetRunManager()->GetUserTrackingAction()));
  }

  /**
   * Register a user action of type RunAction with this class.
   *
   * @param action  User action of type RunAction
   */
  void registerAction(UserAction* trackingAction) {
    trackingActions_.push_back(trackingAction);
  }

 private:
  /// custom user actions to be called before and after processing a track
  std::vector<UserAction*> trackingActions_;

  /** Stores parentage information for all tracks in the event. */
  TrackMap trackMap_;
};  // TrackingAction

}  // namespace simcore::g4user

#endif

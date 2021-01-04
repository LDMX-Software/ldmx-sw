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
   * @param aTrack The Geant4 track.
   */
  void PreUserTrackingAction(const G4Track* aTrack);

  /**
   * Implementation of post-tracking action.
   * @param aTrack The Geant4 track.
   */
  void PostUserTrackingAction(const G4Track* aTrack);

  /**
   * Get a pointer to the current TrackMap for the event.
   * @return A pointer to the current TrackMap for the event.
   */
  TrackMap* getTrackMap() { return &trackMap_; }

  /**
   * Process one track.
   * @param aTrack The track to process.
   * @note
   * <ul>
   * <li>Creates a new trajectory if track should be saved.</li>
   * <li>Copies gen status setting from primary particle to the trajectory.</li>
   * <li>Saves parentage in the TrackMap for all processed tracks.</li>
   * </ul>
   */
  void processTrack(const G4Track* aTrack);

  /**
   * Store a Trajectory for the given G4Track.
   * @param aTrack The Geant4 track.
   */
  void storeTrajectory(const G4Track* aTrack);

  /**
   * Get a pointer to the current UserTrackingAction from the G4RunManager.
   * @return A pointer to the current UserTrackingAction.
   */
  static UserTrackingAction* getUserTrackingAction() {
    return static_cast<UserTrackingAction*>(const_cast<G4UserTrackingAction*>(
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
  ///
  std::vector<UserAction*> trackingActions_;

  /** Stores parentage information for all tracks in the event. */
  TrackMap trackMap_;
};
}  // namespace ldmx

#endif

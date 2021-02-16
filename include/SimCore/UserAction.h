
#ifndef SIMCORE_USERACTION_H
#define SIMCORE_USERACTION_H

/*~~~~~~~~~~~~~~~~*/
/*   C++ StdLib   */
/*~~~~~~~~~~~~~~~~*/
#include <iostream>
#include <map>
#include <string>
#include <vector>

/*~~~~~~~~~~~~*/
/*   Geant4   */
/*~~~~~~~~~~~~*/
#include "G4EventManager.hh"
#include "G4UserStackingAction.hh"

/*~~~~~~~~~~~~~~~*/
/*   Framework   */
/*~~~~~~~~~~~~~~~*/
#include "Framework/Configure/Parameters.h"
#include "SimCore/UserEventInformation.h"

// Forward Declarations
class G4Event;
class G4Run;
class G4Step;
class G4Track;

namespace simcore {

/// Enum for each of the user action types.
enum TYPE { RUN = 1, EVENT, TRACKING, STEPPING, STACKING, NONE };

// Forward declarations
class UserAction;

typedef UserAction* UserActionBuilder(
    const std::string& name, framework::config::Parameters& parameters);

/**
 * @class UserAction
 * @brief Interface that defines a user action.
 */
class UserAction {
 public:
  /**
   * Constructor.
   *
   * @param name Name given the to class instance.
   */
  UserAction(const std::string& name,
             framework::config::Parameters& parameters);

  /// Destructor
  virtual ~UserAction();

  /**
   * Method used to register a user action with the manager.
   *
   * @param className Name of the class instance
   * @param builder The builder used to create and instance of this class.
   */
  static void declare(const std::string& className, UserActionBuilder* builder);

  /**
   * Method called at the beginning of every event.
   *
   * TYPE::EVENT
   *
   * @param event Geant4 event object.
   */
  virtual void BeginOfEventAction(const G4Event*){};

  /**
   * Method called at the end of every event.
   *
   * TYPE::EVENT
   *
   * @param event Geant4 event object.
   */
  virtual void EndOfEventAction(const G4Event*){};

  /**
   * Method called at the beginning of a run.
   *
   * TYPE::RUN
   *
   * @param run Current Geant4 run object.
   */
  virtual void BeginOfRunAction(const G4Run*){};

  /**
   * Method called at the end of a run.
   *
   * TYPE::RUN
   *
   * @param run Current Geant4 run object.
   */
  virtual void EndOfRunAction(const G4Run*){};

  /**
   * Method called before the UserTrackingAction.
   *
   * TYPE::TRACKING
   *
   * @param track current Geant4 track
   */
  virtual void PreUserTrackingAction(const G4Track*){};

  /**
   * Method called after the UserTrackingAction.
   *
   * TYPE::TRACKING
   *
   * @param track current Geant4 track
   */
  virtual void PostUserTrackingAction(const G4Track*){};

  /**
   * Method called after each simulation step.
   *
   * TYPE::STEPPING
   *
   * @param current Geant4 step
   */
  virtual void stepping(const G4Step*){};

  /**
   * Method called when a track is updated
   *
   * TYPE::STEPPING
   *
   * @param current Geant4 track
   * @param current tracks' classification
   */
  virtual G4ClassificationOfNewTrack ClassifyNewTrack(
      const G4Track*, const G4ClassificationOfNewTrack& cl) {
    return cl;
  };

  /**
   * Method called at the beginning of a new stage
   *
   * TYPE::STACKING
   */
  virtual void NewStage(){};

  /**
   * Method called at the beginning of a new event
   *
   * TYPE::STACKING
   */
  virtual void PrepareNewEvent(){};

  /**
   * @return The user action types
   *
   * Must be defined by any UserActions so that we know what functions to call.
   */
  virtual std::vector<TYPE> getTypes() = 0;

 protected:
  /**
   * Get a handle to the event information
   *
   * This is static just to point out that it doesn't
   * depend on any of the member variables of this class.
   * It is just a helper function for shortening any
   * code that interacts with our event information.
   *
   * @returns pointer to the current event information
   */
  static UserEventInformation* getEventInfo() {
    return static_cast<UserEventInformation*>(
        G4EventManager::GetEventManager()->GetUserInformation());
  }

 protected:
  /// Name of the UserAction
  std::string name_{""};

  /// The set of parameters used to configure this class
  framework::config::Parameters parameters_;

};  // UserAction

}  // namespace simcore

#define DECLARE_ACTION(NS, CLASS)                                           \
  simcore::UserAction* CLASS##Builder(                                      \
      const std::string& name, framework::config::Parameters& parameters) { \
    return new NS::CLASS(name, parameters);                                 \
  }                                                                         \
  __attribute((constructor(205))) static void CLASS##Declare() {            \
    simcore::UserAction::declare(                                           \
        std::string(#NS) + "::" + std::string(#CLASS), &CLASS##Builder);    \
  }

#endif  // SIMCORE_USERACTION_H

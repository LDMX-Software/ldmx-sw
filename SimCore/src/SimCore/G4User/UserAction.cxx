
#include "SimCore/G4User/UserAction.h"

#include "SimCore/G4User/TrackingAction.h"

/*~~~~~~~~~~~~*/
/*   Geant4   */
/*~~~~~~~~~~~~*/
#include "G4Event.hh"
#include "G4Run.hh"
#include "G4Step.hh"
#include "G4Track.hh"

namespace simcore {

UserAction::UserAction(const std::string& name,
                       framework::config::Parameters& parameters)
    : name_{name},
      parameters_{parameters},
      the_log_{::framework::logging::makeLogger(name)} {}

UserEventInformation* UserAction::getEventInfo() const {
  return static_cast<UserEventInformation*>(
      G4EventManager::GetEventManager()->GetUserInformation());
}

const std::map<int, ldmx::SimParticle>& UserAction::getCurrentParticleMap()
    const {
  return g4user::TrackingAction::get()->getTrackMap().getParticleMap();
}

void UserAction::abortEvent(const std::string& reason) const {
  ldmx_log(trace)
      << "("
      << G4EventManager::GetEventManager()->GetConstCurrentEvent()->GetEventID()
      << ") " << reason << " Aborting event.";

  G4RunManager::GetRunManager()->AbortEvent();
  return;
}

DEFINE_FACTORY(UserAction);

}  // namespace simcore

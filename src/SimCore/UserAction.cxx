
#include "SimCore/UserAction.h"

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
                       framework::config::Parameters& parameters) {
  name_ = name;
  parameters_ = parameters;
}

UserAction::~UserAction() {}

static const std::map<int,ldmx::SimParticle>& getCurrentParticleMap() {
  return g4user::TrackingAction::get()->getTrackMap().getParticleMap();
}

}  // namespace simcore

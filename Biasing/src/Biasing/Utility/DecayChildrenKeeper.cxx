#include "Biasing/Utility/DecayChildrenKeeper.h"

/*~~~~~~~~~~~~*/
/*   Geant4   */
/*~~~~~~~~~~~~*/
#include "G4Track.hh"
#include "G4VProcess.hh"

/*~~~~~~~~~~~~~*/
/*   SimCore   */
/*~~~~~~~~~~~~~*/
#include "SimCore/UserTrackInformation.h"
#include "SimCore/UserTrackingAction.h"

namespace biasing {
namespace utility {

DecayChildrenKeeper::DecayChildrenKeeper(const std::string& name,
                                       framework::config::Parameters& parameters)
    : simcore::UserAction(name, parameters) {
  parents_ = parameters.getParameter<std::vector<int>>("parents");
}

DecayChildrenKeeper::~DecayChildrenKeeper() {}

void DecayChildrenKeeper::PostUserTrackingAction(const G4Track* track) {
  const auto& particle_map{
    simcore::UserTrackingAction::getUserTrackingAction()->getTrackMap()->getParticleMap()};
  if (particle_map.find(track->GetParentID()) != particle_map.end()) {
    const int& parent_pdg{particle_map.at(track->GetParentID()).getPdgID()};
    for (const int& parent : parents_) {
      if (parent_pdg == parent) {
        simcore::UserTrackInformation::get(track)->setSaveFlag(true);
        break;
      } //parent is an interesting one
    } // loop through interesting parent options
  } // parent is in particle map
}

}  // namespace utility
}  // namespace biasing

DECLARE_ACTION(biasing::utility, DecayChildrenKeeper)

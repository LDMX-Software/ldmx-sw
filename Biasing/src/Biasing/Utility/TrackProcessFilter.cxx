#include "Biasing/Utility/TrackProcessFilter.h"

/*~~~~~~~~~~~~*/
/*   Geant4   */
/*~~~~~~~~~~~~*/
#include "G4Track.hh"
#include "G4VProcess.hh"

/*~~~~~~~~~~~~~*/
/*   SimCore   */
/*~~~~~~~~~~~~~*/
#include "SimCore/UserTrackInformation.h"

namespace biasing {
namespace utility {

TrackProcessFilter::TrackProcessFilter(const std::string& name,
                                       framework::config::Parameters& parameters)
    : simcore::UserAction(name, parameters) {
  process_ = parameters.getParameter<std::string>("process");
}

TrackProcessFilter::~TrackProcessFilter() {}

void TrackProcessFilter::PostUserTrackingAction(const G4Track* track) {
  if (const G4VProcess * process{track->GetCreatorProcess()}; process) {
    auto name{process->GetProcessName()};
    auto trackInfo{simcore::UserTrackInformation::get(track)};
    if (name.contains(process_))
      trackInfo->setSaveFlag(true);
  }  // does this track have a creator process
}

}  // namespace utility
}  // namespace biasing

DECLARE_ACTION(biasing::utility, TrackProcessFilter)

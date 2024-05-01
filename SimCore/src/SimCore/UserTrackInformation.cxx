#include "SimCore/UserTrackInformation.h"

/*~~~~~~~~~~~~~~~~*/
/*   C++ StdLib   */
/*~~~~~~~~~~~~~~~~*/
#include <iostream>

namespace simcore {

UserTrackInformation* UserTrackInformation::get(const G4Track* track) {
  if (!track->GetUserInformation()) {
    const_cast<G4Track*>(track)->SetUserInformation(new UserTrackInformation);
  }
  return dynamic_cast<UserTrackInformation*>(track->GetUserInformation());
}

void UserTrackInformation::initialize(const G4Track* track) {
  initialMomentum_ = track->GetMomentum();
  vertexVolume_ = track->GetLogicalVolumeAtVertex()->GetName();
  vertex_time_ = track->GetGlobalTime();
}
void UserTrackInformation::Print() const {
  std::cout << "Saving track: " << saveFlag_ << "\n"
            << "Is brem candidate: " << isBremCandidate_ << "\n"
            << std::endl;
}
}  // namespace simcore

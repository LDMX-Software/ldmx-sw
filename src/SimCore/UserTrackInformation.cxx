#include "SimCore/UserTrackInformation.h"

/*~~~~~~~~~~~~~~~~*/
/*   C++ StdLib   */
/*~~~~~~~~~~~~~~~~*/
#include <iostream>

namespace simcore {

UserTrackInformation::UserTrackInformation(G4Track* track) {
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

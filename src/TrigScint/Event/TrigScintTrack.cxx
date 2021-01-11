#include "TrigScint/Event/TrigScintTrack.h"

/*~~~~~~~~~~~~~~~~*/
/*   C++ StdLib   */
/*~~~~~~~~~~~~~~~~*/
#include <iostream>

ClassImp(trigscint::event::TrigScintTrack)

namespace trigscint {
namespace event {
void TrigScintTrack::Clear() {
  centroid_ = 0;
  residual_ = 0;
}

void TrigScintTrack::Print() const {
  std::cout << "TrigScintTrack { "
            << " channel centroid: " << getCentroid()
            << ",  residual: " << getResidual() << " }" << std::endl;
}
} // namespace event
} // namespace trigscint

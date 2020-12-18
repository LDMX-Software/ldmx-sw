#include "TrigScint/Event/TrigScintTrack.h"

/*~~~~~~~~~~~~~~~~*/
/*   C++ StdLib   */
/*~~~~~~~~~~~~~~~~*/
#include <iostream>

ClassImp(ldmx::TrigScintTrack)

namespace ldmx {

void TrigScintTrack::Clear() {
  centroid_ = 0;
  residual_ = 0;
}

void TrigScintTrack::Print() const {
  std::cout << "TrigScintTrack { "
            << " channel centroid: " << getCentroid()
            << ",  residual: " << getResidual() << " }" << std::endl;
}
} // namespace ldmx

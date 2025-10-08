#include "TrigScint/Event/TrigScintTrack.h"

/*~~~~~~~~~~~~~~~~*/
/*   C++ StdLib   */
/*~~~~~~~~~~~~~~~~*/
#include <iostream>

ClassImp(ldmx::TrigScintTrack);

namespace ldmx {
void TrigScintTrack::clear() {
  centroid_ = 0;
  residual_ = 0;
}

std::ostream& operator<<(std::ostream& o, const TrigScintTrack& c) {
  return o << "TrigScintTrack { " << " channel centroid: " << c.getCentroid()
           << ",  residual: " << c.getResidual() << " }";
}
}  // namespace ldmx

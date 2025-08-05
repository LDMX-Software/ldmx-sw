#include "Recon/Event/BeamElectronTruth.h"

// STL
#include <iostream>

ClassImp(ldmx::BeamElectronTruth);

namespace ldmx {
void BeamElectronTruth::clear() {
  x_ = -999;
  y_ = -999;
  z_ = -999;
  px_ = -999;
  py_ = -999;
  pz_ = -999;
  binnedX_ = -999;
  binnedY_ = -999;
}
std::ostream& operator<<(std::ostream& o, const BeamElectronTruth& c) {
  return o << " { " << "(x: " << c.x_ << ", y: " << c.y_ << ", z: " << c.z_
           << "); (binned X: " << c.binnedX_ << ", binned Y: " << c.binnedY_
           << "); (px: " << c.px_ << ", py: " << c.py_ << ", pz: " << c.pz_
           << ")";
}
}  // namespace ldmx

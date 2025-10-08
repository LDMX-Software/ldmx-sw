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
  binned_x_ = -999;
  binned_y_ = -999;
}
std::ostream& operator<<(std::ostream& o, const BeamElectronTruth& c) {
  return o << " { " << "(x_: " << c.x_ << ", y_: " << c.y_ << ", z_: " << c.z_
           << "); (binned X: " << c.binned_x_ << ", binned Y: " << c.binned_y_
           << "); (px: " << c.px_ << ", py: " << c.py_ << ", pz: " << c.pz_
           << ")";
}
}  // namespace ldmx

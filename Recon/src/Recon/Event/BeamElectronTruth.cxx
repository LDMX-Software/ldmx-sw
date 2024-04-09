#include "Recon/Event/BeamElectronTruth.h"

// STL
#include <iostream>

ClassImp(ldmx::BeamElectronTruth)

    namespace ldmx {
  void BeamElectronTruth::Clear() {
    x_ = -999;
    y_ = -999;
    z_ = -999;
    px_ = -999;
    py_ = -999;
    pz_ = -999;
    binnedX_ = -999;
    binnedY_ = -999;
  }

  void BeamElectronTruth::Print() const {
    std::cout << "BeamElectronTruth { "
              << "(x: " << x_ << ", y: " << y_ << ", z: " << z_
              << "); (binned X: " << binnedX_ << ", binned Y: " << binnedY_
              << "); (px: " << px_ << ", py: " << py_ << ", pz: " << pz_ << ")"
              << std::endl;
  }
}  // namespace ldmx

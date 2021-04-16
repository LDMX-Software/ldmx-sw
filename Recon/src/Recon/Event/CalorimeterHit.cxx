#include "Recon/Event/CalorimeterHit.h"

// STL
#include <iostream>

ClassImp(ldmx::CalorimeterHit)

    namespace ldmx {
  void CalorimeterHit::Clear() {
    id_ = 0;
    amplitude_ = 0;
    energy_ = 0;
    time_ = 0;
    xpos_ = 0;
    ypos_ = 0;
    zpos_ = 0;
    isNoise_ = false;
  }

  void CalorimeterHit::Print() const {
    std::cout << "CalorimeterHit { "
              << "id: " << std::hex << id_ << std::dec
              << ",  energy: " << energy_ << "MeV, time: " << time_
              << "ns, amplitude: " << amplitude_ << "}" << std::endl;
  }
}  // namespace ldmx

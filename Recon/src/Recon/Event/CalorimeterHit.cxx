#include "Recon/Event/CalorimeterHit.h"

// STL
#include <iostream>

ClassImp(ldmx::CalorimeterHit);

namespace ldmx {
void CalorimeterHit::clear() {
  id_ = 0;
  amplitude_ = 0;
  energy_ = 0;
  time_ = 0;
  xpos_ = 0;
  ypos_ = 0;
  zpos_ = 0;
  isNoise_ = false;
}

std::ostream& operator<<(std::ostream& o, const CalorimeterHit& c) {
  return o << "CalorimeterHit { " << "id: " << std::hex << c.id_ << std::dec
           << ",  energy: " << c.energy_ << "MeV, time: " << c.time_
           << "ns, amplitude: " << c.amplitude_ << "}";
}
}  // namespace ldmx

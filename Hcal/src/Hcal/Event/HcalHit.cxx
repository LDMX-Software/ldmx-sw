#include "Hcal/Event/HcalHit.h"

// STL
#include <iostream>

ClassImp(ldmx::HcalHit);

namespace ldmx {
void HcalHit::clear() {
  ldmx::CalorimeterHit::clear();
  pe_ = 0;
  minpe_ = -99;
}

std::ostream& operator<<(std::ostream& o, const HcalHit& c) {
  return o << "HcalHit { " << "id: " << std::hex << c.getID() << std::dec
           << ",  energy: " << c.getEnergy() << "MeV, time: " << c.getTime()
           << "ns, amplitude: " << c.getAmplitude() << ", pe: " << c.getPE()
           << "}";
}
}  // namespace ldmx

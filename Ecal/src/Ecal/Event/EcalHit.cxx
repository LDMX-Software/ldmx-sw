#include "Ecal/Event/EcalHit.h"

// STL
#include <iostream>

ClassImp(ldmx::EcalHit);

namespace ldmx {
void EcalHit::clear() { CalorimeterHit::clear(); }

std::ostream& operator<<(std::ostream& o, const EcalHit& hit) {
  return o << "EcalHit { " << "id: " << std::hex << hit.getID() << std::dec
           << ",  energy: " << hit.getEnergy() << "MeV, time: " << hit.getTime()
           << "ns, amplitude: " << hit.getAmplitude() << "}";
}
}  // namespace ldmx

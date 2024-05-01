#include "Ecal/Event/EcalHit.h"

// STL
#include <iostream>

ClassImp(ldmx::EcalHit)

    namespace ldmx {
  void EcalHit::Clear() { CalorimeterHit::Clear(); }

  void EcalHit::Print() const {
    std::cout << "EcalHit { "
              << "id: " << std::hex << getID() << std::dec
              << ",  energy: " << getEnergy() << "MeV, time: " << getTime()
              << "ns, amplitude: " << getAmplitude() << "}" << std::endl;
  }
}  // namespace ldmx

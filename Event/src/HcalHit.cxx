#include "Event/HcalHit.h"

// STL
#include <iostream>

ClassImp(ldmx::HcalHit)

namespace ldmx {

void HcalHit::Clear(Option_t *option) {
    CalorimeterHit::Clear();
    pe_ = 0;
}

void HcalHit::Print(Option_t *option) const {
    std::cout << "HcalHit { " << "id: " << std::hex << getID() << std::dec
            << ",  energy: " << getEnergy() << "MeV, time: " << getTime()
            << "ns, amplitude: " << getAmplitude() << ", pe: " << getPE() << "}" << std::endl;

}
}

#include "Event/HcalStripHit.h"

// STL
#include <iostream>

ClassImp(ldmx::HcalStripHit)

namespace ldmx {

    void HcalStripHit::Clear(Option_t *option) {
        CalorimeterHit::Clear();
        pe_ = 0;
    }

    void HcalStripHit::Print(Option_t *option) const {
        std::cout << "HcalStripHit { " << "id: " << std::hex << getID() << std::dec
                << ",  energy: " << getEnergy() << "MeV, time: " << getTime()
                << "ns, amplitude: " << getAmplitude() << ", pe: " << getPE() << "}" << std::endl;

    }
}

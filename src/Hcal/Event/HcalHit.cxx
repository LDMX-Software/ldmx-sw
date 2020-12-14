#include "Hcal/Event/HcalHit.h"

// STL
#include <iostream>

ClassImp(ldmx::HcalHit)

namespace ldmx {

    void HcalHit::Clear() {
        CalorimeterHit::Clear();
        pe_ = 0;
        minpe_ = -99;
    }

    void HcalHit::Print() const {
        std::cout << "HcalHit { " << "id: " << std::hex << getID() << std::dec
                << ",  energy: " << getEnergy() << "MeV, time: " << getTime()
                << "ns, amplitude: " << getAmplitude() << ", pe: " << getPE() << "}" << std::endl;

    }
}

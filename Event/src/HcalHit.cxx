#include "Event/HcalHit.h"

ClassImp(ldmx::HcalHit)

namespace ldmx {

    void HcalHit::Clear() {
        CalorimeterHit::Clear();
        pe_ = 0;
    }

    void HcalHit::Print(std::ostream &o) const {
        o << "HcalHit { " << "id: " << std::hex << getID() << std::dec
          << ",  energy: " << getEnergy() << "MeV, time: " << getTime()
          << "ns, amplitude: " << getAmplitude() << ", pe: " << getPE() << "}";
    }
}

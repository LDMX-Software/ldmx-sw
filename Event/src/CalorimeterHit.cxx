#include "Event/CalorimeterHit.h"

ClassImp(ldmx::CalorimeterHit)

namespace ldmx {

    void CalorimeterHit::Clear() {

        id_        = 0;
        amplitude_ = 0;
        energy_    = 0;
        time_      = 0;
        xpos_      = 0;
        ypos_      = 0;
        zpos_      = 0;
        isNoise_   = false;

    }

    void CalorimeterHit::Print(std::ostream &o) const {
        o << "CalorimeterHit { " << "id: " << std::hex << id_ << std::dec
          << ",  energy: " << energy_ << "MeV, time: " << time_
          << "ns, amplitude: " << amplitude_ << "}";
    }

    int CalorimeterHit::getLayer() const {
        return (getID() & 0xFF0) >> 4; // depends on internal knowledge of Detector Id, but efficiency is important for this method
    }

}

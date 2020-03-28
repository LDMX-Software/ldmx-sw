#include "Event/EcalHit.h"

// STL
#include <iostream>

ClassImp(ldmx::EcalHit)

namespace ldmx {

    void EcalHit::Clear() { 
        CalorimeterHit::Clear(); 
    }

    void EcalHit::Print() const {
        std::cout << "EcalHit { " << "id: " << std::hex << getID() << std::dec
                << ",  energy: " << getEnergy() << "MeV, time: " << getTime()
                << "ns, amplitude: " << getAmplitude() << "}" << std::endl;

    }

    int EcalHit::getCell() const {
        return (getID() >> 12); // depends on internal knowledge of Detector Id, but efficiency is important for this method
    }
}

#include "Event/EcalHit.h"

// STL
#include <iostream>

ClassImp(event::EcalHit)

namespace event {

void EcalHit::Print(Option_t *option) const {
    std::cout << "EcalHit { " << "id: " << std::hex << getID() << std::dec
            << ",  energy: " << getEnergy() << "MeV, time: " << getTime()
            << "ns, amplitude: " << getAmplitude() << "}" << std::endl;

}

int EcalHit::getCell() const {
    return (getID() >> 12); // depends on internal knowledge of Detector Id, but efficiency is important for this method
}
}

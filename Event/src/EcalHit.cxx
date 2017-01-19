#include "Event/EcalHit.h"

// STL
#include <iostream>

ClassImp(event::EcalHit)

namespace event {

    void EcalHit::Print(Option_t *option)  const {
	std::cout << "EcalHit { " << "id: " << getID() << ",  energy: " << getEnergy()
		  << "MeV, time: " << getTime() << "ns, amplitude: " << getAmplitude() << std::endl;
	
    }
}

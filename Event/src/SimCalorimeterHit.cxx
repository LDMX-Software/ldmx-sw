#include "Event/SimCalorimeterHit.h"

// STL
#include <iostream>

ClassImp(event::SimCalorimeterHit)

namespace event {

void SimCalorimeterHit::Print(Option_t *option) const {
    std::cout << "SimCalorimeterHit { " << "id: " << id << ", " << "edep: " << edep << ", "
            "position: ( " << x << ", " << y << ", " << z << " ) }" << std::endl;
}

}

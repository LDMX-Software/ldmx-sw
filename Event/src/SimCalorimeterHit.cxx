#include "Event/SimCalorimeterHit.h"

// STL
#include <iostream>

ClassImp(event::SimCalorimeterHit)

namespace event {

void SimCalorimeterHit::Print(Option_t *option) const {
    std::cout << "SimCalorimeterHit { " << "id: " << id_ << ", " << "edep: " << edep_ << ", "
            "position: ( " << x_ << ", " << y_ << ", " << z_ << " ) }" << std::endl;
}

}

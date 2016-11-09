#include "Event/SimCalorimeterHit.h"

// STL
#include <iostream>

ClassImp(event::SimCalorimeterHit)

namespace event {

SimCalorimeterHit::SimCalorimeterHit()
    : TObject() {
}

SimCalorimeterHit::~SimCalorimeterHit() {
    Clear();
}

void SimCalorimeterHit::Clear(Option_t *option) {
    TObject::Clear();
}

void SimCalorimeterHit::Print(Option_t *option) const {
    std::cout << "SimCalorimeterHit { " << "id: " << id_ << ", " << "edep: " << edep_ << ", "
            "position: ( " << x_ << ", " << y_ << ", " << z_ << " ) }" << std::endl;
}

}

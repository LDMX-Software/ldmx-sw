#include "Event/ReadoutCalorimeterHit.h"

// STL
#include <iostream>

ClassImp(event::ReadoutCalorimeterHit)

namespace event {

ReadoutCalorimeterHit::ReadoutCalorimeterHit()
    : TObject() {
}

ReadoutCalorimeterHit::~ReadoutCalorimeterHit() {
    Clear();
}

void ReadoutCalorimeterHit::Clear(Option_t *option) {
    TObject::Clear();
}

void ReadoutCalorimeterHit::Print(Option_t *option) const {
    std::cout << "ReadoutCalorimeterHit { " << "id: " << id_ << ", " << "edep: " << edep_ << ", "
            "position: ( " << x_ << ", " << y_ << ", " << z_ << " ) }" << std::endl;
}

}

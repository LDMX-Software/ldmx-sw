#include "Event/SimCalorimeterHit.h"

// STL
#include <iostream>

ClassImp(SimCalorimeterHit)

SimCalorimeterHit::SimCalorimeterHit() :
        id(0), edep(0.) {
    position[0] = 0.;
    position[1] = 0.;
    position[2] = 0.;
}

SimCalorimeterHit::~SimCalorimeterHit() {
}

void SimCalorimeterHit::Print(Option_t *option) const {
    std::cout << "SimCalorimeterHit { " << "id: " << id << ", " << "edep: " << edep << ", "
            "position: ( " << position[0] << ", " << position[1] << ", " << position[2] << " ) }" << std::endl;
}

int SimCalorimeterHit::getId() {
    return id;
}

double SimCalorimeterHit::getEdep() {
    return edep;
}

double* SimCalorimeterHit::getPosition() {
    return position;
}

void SimCalorimeterHit::setId(long id) {
    this->id = id;
}

void SimCalorimeterHit::setEdep(double edep) {
    this->edep = edep;
}

void SimCalorimeterHit::setPosition(double x, double y, double z) {
    position[0] = x;
    position[1] = y;
    position[2] = z;
}

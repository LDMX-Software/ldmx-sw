#include "Event/SimCalorimeterHit.h"

// STL
#include <iostream>

ClassImp(event::SimCalorimeterHit)

namespace event {

SimCalorimeterHit::SimCalorimeterHit() :
        id(0),
        edep(0.),
        time(0.) {
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

int SimCalorimeterHit::getID() {
    return id;
}

double SimCalorimeterHit::getEdep() {
    return edep;
}

double* SimCalorimeterHit::getPosition() {
    return position;
}

float SimCalorimeterHit::getTime() {
    return time;
}

SimParticle* SimCalorimeterHit::getSimParticle() {
    return (SimParticle*) simParticle.GetObject();
}

void SimCalorimeterHit::setID(long id) {
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

void SimCalorimeterHit::setTime(float time) {
    this->time = time;
}

void SimCalorimeterHit::setSimParticle(SimParticle* aSimParticle) {
    simParticle = aSimParticle;
}

}

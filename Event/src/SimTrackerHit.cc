#include "Event/SimTrackerHit.h"

#include <iostream>
#include <bitset>

ClassImp(SimTrackerHit)

SimTrackerHit::SimTrackerHit() :
    id(0),
    edep(0.),
    time(0.) {

    startPosition[0] = 0.;
    startPosition[1] = 0.;
    startPosition[2] = 0.;

    endPosition[0] = 0.;
    endPosition[1] = 0.;
    endPosition[2] = 0.;

    momentum[0] = 0.;
    momentum[1] = 0.;
    momentum[2] = 0.;
}

SimTrackerHit::~SimTrackerHit() {

}

void SimTrackerHit::Print(Option_t *option) const {
    std::cout << "SimTrackerHit { " <<
            "id: " << std::bitset<32>(id) << ", " <<
            "startPosition: ( " << startPosition[0] << ", " << startPosition[1] << ", " << startPosition[2] << " ), " <<
            "endPosition: ( " << endPosition[0] << ", " << endPosition[1] << ", " << endPosition[2] << " ), " <<
            "edep: " << edep << ", " <<
            "time: " << time << ", " <<
            "momentum: ( " << momentum[0] << ", " << momentum[1] << ", " << momentum[2] << " )" <<
            " }" << std::endl;
}

int SimTrackerHit::getId() {
    return id;
}

double* SimTrackerHit::getStartPosition() {
    return startPosition;
}

double* SimTrackerHit::getEndPosition() {
    return endPosition;
}

float  SimTrackerHit::getEdep() {
    return edep;
}

float SimTrackerHit::getTime() {
    return time;
}

float* SimTrackerHit::getMomentum() {
    return momentum;
}

SimParticle* SimTrackerHit::getSimParticle() {
    return (SimParticle*) simParticle.GetObject();
}

void SimTrackerHit::setId(long id) {
    this->id = id;
}

void SimTrackerHit::setStartPosition(double x, double y, double z) {
    startPosition[0] = x;
    startPosition[1] = y;
    startPosition[2] = z;
}

void SimTrackerHit::setEndPosition(double x, double y, double z) {
    endPosition[0] = x;
    endPosition[1] = y;
    endPosition[2] = z;
}

void SimTrackerHit::setEdep(float edep) {
    this->edep = edep;
}

void SimTrackerHit::setTime(float time) {
    this->time = time;
}

void SimTrackerHit::setMomentum(float px, float py, float pz) {
    momentum[0] = px;
    momentum[1] = py;
    momentum[2] = pz;
}

void SimTrackerHit::setSimParticle(SimParticle* aSimParticle) {
    this->simParticle = aSimParticle;
}

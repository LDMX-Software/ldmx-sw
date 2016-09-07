#include "Event/SimTrackerHit.h"

#include <iostream>

ClassImp(SimTrackerHit)

SimTrackerHit::SimTrackerHit() :
    _id(0),
    _edep(0.),
    _time(0.) {

    _startPosition[0] = 0.;
    _startPosition[1] = 0.;
    _startPosition[2] = 0.;

    _endPosition[0] = 0.;
    _endPosition[1] = 0.;
    _endPosition[2] = 0.;

    _momentum[0] = 0.;
    _momentum[1] = 0.;
    _momentum[2] = 0.;
}

SimTrackerHit::~SimTrackerHit() {

}

void SimTrackerHit::Print(Option_t *option) const {
    std::cout << "SimTrackerHit { " <<
            "id: " << _id << ", " <<
            "startPosition: ( " << _startPosition[0] << ", " << _startPosition[1] << ", " << _startPosition[2] << " ), " <<
            "endPosition: ( " << _endPosition[0] << ", " << _endPosition[1] << ", " << _endPosition[2] << " ), " <<
            "edep: " << _edep << ", " <<
            "time: " << _time << ", " <<
            "momentum: ( " << _momentum[0] << ", " << _momentum[1] << ", " << _momentum[2] << " )" <<
            " }" << std::endl;
}

long SimTrackerHit::id() {
    return _id;
}

double* SimTrackerHit::startPosition() {
    return _startPosition;
}

double* SimTrackerHit::endPosition() {
    return _endPosition;
}

float  SimTrackerHit::edep() {
    return _edep;
}

float SimTrackerHit::time() {
    return _time;
}

float* SimTrackerHit::momentum() {
    return _momentum;
}

void SimTrackerHit::setId(long id) {
    _id = id;
}

void SimTrackerHit::setStartPosition(double x, double y, double z) {
    _startPosition[0] = x;
    _startPosition[1] = y;
    _startPosition[2] = z;
}

void SimTrackerHit::setEndPosition(double x, double y, double z) {
    _endPosition[0] = x;
    _endPosition[1] = y;
    _endPosition[2] = z;
}

void SimTrackerHit::setEdep(float edep) {
    _edep = edep;
}

void SimTrackerHit::setTime(float time) {
    _time = time;
}

void SimTrackerHit::setMomentum(float px, float py, float pz) {
    _momentum[0] = px;
    _momentum[1] = py;
    _momentum[2] = pz;
}

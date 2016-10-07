#include "Event/SimTrackerHit.h"

#include <iostream>
#include <bitset>

ClassImp(SimTrackerHit)

SimTrackerHit::SimTrackerHit() 
    : TObject() {

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

/*
double* SimTrackerHit::getStartPosition() {
    return startPosition;
}

double* SimTrackerHit::getEndPosition() {
    return endPosition;
}
*/

std::vector<double> SimTrackerHit::getMomentum() {
    return std::vector<double> momentum{px, py, pz};
}

SimParticle* SimTrackerHit::getSimParticle() {
    return (SimParticle*) simParticle.GetObject();
}

/*void SimTrackerHit::setStartPosition(double x, double y, double z) {
    startPosition[0] = x;
    startPosition[1] = y;
    startPosition[2] = z;
}

void SimTrackerHit::setEndPosition(double x, double y, double z) {
    endPosition[0] = x;
    endPosition[1] = y;
    endPosition[2] = z;
}*/

void SimTrackerHit::setMomentum(const float px, const float py, const float pz) {
    this->px = px;
    this->py = py; 
    this->pz = pz; 
}

void SimTrackerHit::setSimParticle(SimParticle* aSimParticle) {
    this->simParticle = aSimParticle;
}

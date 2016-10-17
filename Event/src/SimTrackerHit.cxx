/**
 *
 * @file SimTrackerHit.cc
 * @brief Class used to encapsulate information from a hit in a tracking 
 *        detector.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 *
 */

#include "Event/SimTrackerHit.h"

ClassImp(SimTrackerHit)

SimTrackerHit::SimTrackerHit() 
    : TObject() {
}

SimTrackerHit::~SimTrackerHit() {
    Clear();
}

void SimTrackerHit::Print(Option_t *option) const {
    std::cout << "SimTrackerHit { " << "id: " << std::bitset<32>(id) << ", " <<
            "position: ( " << x << ", " << y << ", " << z << " ), " <<
            "edep: " << edep << ", " <<
            "time: " << time << ", " <<
            "momentum: ( " << px << ", " << py << ", " << pz << " )" <<
            " }" << std::endl;
}

void SimTrackerHit::Clear(Option_t*) {
    TObject::Clear(); 
}

std::vector<float> SimTrackerHit::getPosition() const {
    return {x, y, z};  
}

std::vector<double> SimTrackerHit::getMomentum() const {
    return {px, py, pz};
}

int SimTrackerHit::getID() const {
    return id;
}

float SimTrackerHit::getEdep() const {
    return edep;
};

SimParticle* SimTrackerHit::getSimParticle() const {
    return static_cast<SimParticle*>(simParticle.GetObject());
}

void SimTrackerHit::setPosition(const float x, const float y, const float z) { 
    this->x = x; 
    this->y = y; 
    this->z = z; 
}

void SimTrackerHit::setMomentum(const float px, const float py, const float pz) {
    this->px = px;
    this->py = py; 
    this->pz = pz; 
}

void SimTrackerHit::setSimParticle(SimParticle* aSimParticle) {
    this->simParticle = aSimParticle;
}

float SimTrackerHit::getTime() const {
    return time;
}

float SimTrackerHit::getPathLength() const {
    return pathLength;
}

void SimTrackerHit::setID(const long id) {
    this->id = id;
}

void SimTrackerHit::setEdep(const float edep) {
    this->edep = edep;
}

void SimTrackerHit::setTime(const float time) {
    this->time = time;
}

void SimTrackerHit::setPathLength(const float pathLength) {
    this->pathLength = pathLength;
}

#include "Event/SimParticle.h"

ClassImp(SimParticle)

SimParticle::SimParticle() :
    _energy(0.),
    _pdg(0),
    _simStatus(0),
    _genStatus(0),
    _time(0),
    _mass(0) {

    double _vertex[3];
    double _endPoint[3];
    double _momentum[3];
}

SimParticle::~SimParticle() {
}

double SimParticle::energy() {
    return _energy;
}

int SimParticle::pdg() {
    return _pdg;
}

int SimParticle::simStatus() {
    return _simStatus;
}

int SimParticle::genStatus() {
    return _genStatus;
}

float SimParticle::time() {
    return _time;
}

double* SimParticle::vertex() {
    return _vertex;
}

double* SimParticle::endPoint() {
    return _endPoint;
}

double* SimParticle::momentum() {
    return _momentum;
}

double SimParticle::mass() {
    return _mass;
}

std::vector<SimParticle*>& SimParticle::daughters() {
    return _daughters;
}

double SimParticle::setEnergy(double energy) {
    _energy = energy;
}

double SimParticle::setPdg(int pdg) {
    _pdg = pdg;
}

void SimParticle::setSimStatus(int simStatus) {
    _simStatus = simStatus;
}

void SimParticle::setGenStatus(int genStatus) {
    _genStatus = genStatus;
}

void SimParticle::setTime(float time) {
    _time = time;
}

void SimParticle::setVertex(double x, double y, double z) {
    _vertex[0] = x;
    _vertex[1] = y;
    _vertex[2] = z;
}

void SimParticle::setEndPoint(double x, double y, double z) {
    _endPoint[0] = x;
    _endPoint[1] = y;
    _endPoint[2] = y;
}

void SimParticle::setMomentum(double px, double py, double pz) {
    _momentum[0] = px;
    _momentum[1] = py;
    _momentum[2] = pz;
}

void SimParticle::setMass(double mass) {
    _mass = mass;
}

void SimParticle::addDaughter(SimParticle* daughter) {
    _daughters.push_back(daughter);
}

#include "Event/SimParticle.h"

// STL
#include <iostream>

ClassImp(SimParticle)

SimParticle::SimParticle() :
    _energy(0.),
    _pdg(0),
    _simStatus(0),
    _genStatus(0),
    _time(0),
    _mass(0) /*,
    _daughters(new TRefArray()),
    _parents(new TRefArray())*/ {
}

SimParticle::~SimParticle() {
    //delete _daughters;
    //delete _parents;
}

void SimParticle::Print(Option_t *option) const {
    std::cout << "SimParticle { " <<
            "energy: " << _energy << ", " <<
            "pdg: " << _pdg << ", " <<
            "simStatus: " << _simStatus << ", " <<
            "genStatus: " << _genStatus << ", " <<
            "time: " << _time << ", " <<
            "vertex: ( " << _vertex[0] << ", " << _vertex[1] << ", " << _vertex[2] << " ), " <<
            "endPoint: ( " << _endPoint[0] << ", " << _endPoint[1] << ", " << _endPoint[2] << " ), " <<
            "momentum: ( " << _momentum[0] << ", " << _momentum[1] << ", " << _momentum[2] << " ), " <<
            "mass: " << _mass << ", " <<
            //"nDaughters: " << _daughters->GetEntries() <<
            //"nParents: " << _parents->GetEntries() <<
            " }" <<
            std::endl;
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

/*
std::vector<SimParticle*> SimParticle::daughters() {
    std::vector<SimParticle*> daughters;
    for (int iDau = 0; iDau < _daughters->GetEntries(); iDau++) {
        daughters.push_back((SimParticle*) _daughters->At(iDau));
    }
    return daughters;
}

std::vector<SimParticle*> SimParticle::parents() {
    std::vector<SimParticle*> parents;
    for (int iParent = 0; iParent < _parents->GetEntries(); iParent++) {
        parents.push_back((SimParticle*) _parents->At(iParent));
    }
    return parents;
}
*/

void SimParticle::setEnergy(double energy) {
    _energy = energy;
}

void SimParticle::setPdg(int pdg) {
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
    //_daughters->Add(daughter);
}

void SimParticle::addParent(SimParticle* parent) {
    //_parents->Add(parent);
}


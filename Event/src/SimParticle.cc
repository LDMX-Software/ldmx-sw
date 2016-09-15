#include "Event/SimParticle.h"

// STL
#include <iostream>

ClassImp(SimParticle)

SimParticle::SimParticle() :
    energy(0.),
    pdg(0),
    simStatus(0),
    genStatus(0),
    time(0),
    mass(0) /*,
    _daughters(new TRefArray()),
    _parents(new TRefArray())*/ {
}

SimParticle::~SimParticle() {
    //delete _daughters;
    //delete _parents;
}

void SimParticle::Print(Option_t *option) const {
    std::cout << "SimParticle { " <<
            "energy: " << energy << ", " <<
            "pdg: " << pdg << ", " <<
            "simStatus: " << simStatus << ", " <<
            "genStatus: " << genStatus << ", " <<
            "time: " << time << ", " <<
            "vertex: ( " << vertex[0] << ", " << vertex[1] << ", " << vertex[2] << " ), " <<
            "endPoint: ( " << endPoint[0] << ", " << endPoint[1] << ", " << endPoint[2] << " ), " <<
            "momentum: ( " << momentum[0] << ", " << momentum[1] << ", " << momentum[2] << " ), " <<
            "mass: " << mass << ", " <<
            //"nDaughters: " << _daughters->GetEntries() <<
            //"nParents: " << _parents->GetEntries() <<
            " }" <<
            std::endl;
}

double SimParticle::getEnergy() {
    return energy;
}

int SimParticle::getPdg() {
    return pdg;
}

int SimParticle::getSimStatus() {
    return simStatus;
}

int SimParticle::getGenStatus() {
    return genStatus;
}

float SimParticle::getTime() {
    return time;
}

double* SimParticle::getVertex() {
    return vertex;
}

double* SimParticle::getEndPoint() {
    return endPoint;
}

double* SimParticle::getMomentum() {
    return momentum;
}

double SimParticle::getMass() {
    return mass;
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
    this->energy = energy;
}

void SimParticle::setPdg(int pdg) {
    this->pdg = pdg;
}

void SimParticle::setSimStatus(int simStatus) {
    this->simStatus = simStatus;
}

void SimParticle::setGenStatus(int genStatus) {
    this->genStatus = genStatus;
}

void SimParticle::setTime(float time) {
    this->time = time;
}

void SimParticle::setVertex(double x, double y, double z) {
    vertex[0] = x;
    vertex[1] = y;
    vertex[2] = z;
}

void SimParticle::setEndPoint(double x, double y, double z) {
    endPoint[0] = x;
    endPoint[1] = y;
    endPoint[2] = y;
}

void SimParticle::setMomentum(double px, double py, double pz) {
    momentum[0] = px;
    momentum[1] = py;
    momentum[2] = pz;
}

void SimParticle::setMass(double mass) {
    this->mass = mass;
}

void SimParticle::addDaughter(SimParticle* daughter) {
    //_daughters->Add(daughter);
}

void SimParticle::addParent(SimParticle* parent) {
    //_parents->Add(parent);
}


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
    mass(0),
    charge(0),
    daughters(new TRefArray()),
    parents(new TRefArray()) {
}

SimParticle::~SimParticle() {
    daughters->Delete();
    parents->Delete();
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
            "nDaughters: " << daughters->GetEntries() << ", "
            "nParents: " << parents->GetEntries() <<
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

double SimParticle::getCharge() {
    return charge;
}

std::vector<SimParticle*> SimParticle::getDaughters() {
    std::vector<SimParticle*> dauVec;
    for (int iDau = 0; iDau < daughters->GetEntries(); iDau++) {
        dauVec.push_back((SimParticle*) daughters->At(iDau));
    }
    return dauVec;
}

std::vector<SimParticle*> SimParticle::getParents() {
    std::vector<SimParticle*> parVec;
    for (int iParent = 0; iParent < parents->GetEntries(); iParent++) {
        parVec.push_back((SimParticle*) parents->At(iParent));
    }
    return parVec;
}


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

void SimParticle::setCharge(double charge) {
    this->charge = charge;
}

void SimParticle::addDaughter(SimParticle* daughter) {
    daughters->Add(daughter);
}

void SimParticle::addParent(SimParticle* parent) {
    parents->Add(parent);
}


#include "Event/SimParticle.h"

// STL
#include <iostream>

ClassImp(event::SimParticle)

namespace event {

SimParticle::SimParticle()
    : TObject(), daughters_(new TRefArray()), parents_(new TRefArray())
{;}

SimParticle::~SimParticle() {
    TObject::Clear();
    daughters_->Delete();
    parents_->Delete();
}

void SimParticle::Clear(Option_t *option) {
    TObject::Clear();
}

void SimParticle::Print(Option_t *option) const {
    std::cout << "SimParticle { " <<
            "energy: " << energy_ << ", " <<
            "pdg: " << pdg_ << ", " <<
            "genStatus: " << genStatus_ << ", " <<
            "time: " << time_ << ", " <<
            "vertex: ( " << x_ << ", " << y_ << ", " << z_ << " ), " <<
            "endPoint: ( " << endX_ << ", " << endY_ << ", " << endZ_ << " ), " <<
            "momentum: ( " << px_ << ", " << py_ << ", " << pz_ << " ), " <<
            "mass: " << mass_ << ", " <<
            "nDaughters: " << daughters_->GetEntries() << ", "
            "nParents: " << parents_->GetEntries() <<
            " }" << std::endl;
}


std::vector<SimParticle*> SimParticle::getDaughters() {
    std::vector<SimParticle*> dauVec;
    for (int iDau = 0; iDau < daughters_->GetEntries(); iDau++) {
        dauVec.push_back((SimParticle*) daughters_->At(iDau));
    }
    return dauVec;
}

std::vector<SimParticle*> SimParticle::getParents() {
    std::vector<SimParticle*> parVec;
    for (int iParent = 0; iParent < parents_->GetEntries(); iParent++) {
        parVec.push_back((SimParticle*) parents_->At(iParent));
    }
    return parVec;
}

}


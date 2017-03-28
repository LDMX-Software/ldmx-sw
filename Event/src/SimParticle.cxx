#include "Event/SimParticle.h"

// STL
#include <iostream>

ClassImp(ldmx::SimParticle)

namespace ldmx {

SimParticle::SimParticle()
    : TObject(), daughters_(new TRefArray()), parents_(new TRefArray())
{;}

SimParticle::~SimParticle() {
    TObject::Clear();
    delete daughters_;
    delete parents_; 
}

void SimParticle::Clear(Option_t *option) {
    TObject::Clear();

    daughters_->Delete();
    parents_->Delete();

    energy_ = 0;
    pdgID_ = 0;
    genStatus_ = -1;
    time_ = 0;
    x_ = 0;
    y_ = 0;
    z_ = 0;
    endX_ = 0;
    endY_ = 0;
    endZ_ = 0;
    px_ = 0;
    py_ = 0;
    pz_ = 0;
    mass_ = 0;
    charge_ = 0;
}

void SimParticle::Print(Option_t *option) const {
    std::cout << "SimParticle { " <<
            "energy: " << energy_ << ", " <<
            "PDG ID: " << pdgID_ << ", " <<
            "genStatus: " << genStatus_ << ", " <<
            "time: " << time_ << ", " <<
            "vertex: ( " << x_ << ", " << y_ << ", " << z_ << " ), " <<
            "endPoint: ( " << endX_ << ", " << endY_ << ", " << endZ_ << " ), " <<
            "momentum: ( " << px_ << ", " << py_ << ", " << pz_ << " ), " <<
            "mass: " << mass_ << ", " <<
            "nDaughters: " << daughters_->GetEntries() << ", "
            "nParents: " << parents_->GetEntries() << ", "
            "processType: " << processType_ <<
            " }" << std::endl;
}

}


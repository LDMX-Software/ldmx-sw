#include "Event/SimCalorimeterHit.h"

// STL
#include <iostream>

ClassImp(event::SimCalorimeterHit)

namespace event {

SimCalorimeterHit::SimCalorimeterHit()
    : TObject(), simParticleContribs_(new TRefArray) {
}

SimCalorimeterHit::~SimCalorimeterHit() {
    TObject::Clear();
    delete simParticleContribs_;
    simParticleContribs_ = 0;
}

void SimCalorimeterHit::Clear(Option_t *option) {
    TObject::Clear();
    simParticleContribs_->Delete();
}

void SimCalorimeterHit::Print(Option_t *option) const {
    std::cout << "SimCalorimeterHit { " << "id: " << id_ << ",  edep: " << edep_ << ", "
            "position: ( " << x_ << ", " << y_ << ", " << z_ << " ) }" << std::endl;
}

void SimCalorimeterHit::addContrib(SimParticle* simParticle, int pdgCode, float edep, float time) {
    simParticleContribs_->Add(simParticle);
    pdgCodeContribs_.push_back(pdgCode);
    edepContribs_.push_back(edep);
    timeContribs_.push_back(time);
    edep_ += edep;
    if (time < time_ || time_ == 0) {
        time_ = time;
    }
    ++nContribs;
}

SimCalorimeterHit::Contrib SimCalorimeterHit::getContrib(int i) {
    Contrib contrib;
    contrib.particle = (SimParticle*) simParticleContribs_->At(i);
    contrib.edep = edepContribs_[i];
    contrib.time = timeContribs_[i];
    contrib.pdgCode = pdgCodeContribs_[i];
    return contrib;
}

}

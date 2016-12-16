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
    pdgCodeContribs_.clear();
    edepContribs_.clear();
    timeContribs_.clear();

    nContribs_ = 0;
    id_ = 0;
    edep_ = 0;
    x_ = 0;
    y_ = 0;
    z_ = 0;
    time_ = 0;
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
    ++nContribs_;
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

#include "Event/SimTrackerHit.h"

ClassImp(event::SimTrackerHit)

namespace event {

SimTrackerHit::SimTrackerHit()
    : TObject() {
}

SimTrackerHit::~SimTrackerHit() {
    Clear();
}

void SimTrackerHit::Print(Option_t *option) const {
    std::cout << "SimTrackerHit { " << "id: " << id_ << ", " <<
        "layerID: " << layerID_ << ", " <<
        "position: ( " << x_ << ", " << y_ << ", " << z_ << " ), " <<
        "edep: " << edep_ << ", " <<
        "time: " << time_ << ", " <<
        "momentum: ( " << px_ << ", " << py_ << ", " << pz_ << " )" <<
        " }" << std::endl;
}

void SimTrackerHit::Clear(Option_t*) {
    TObject::Clear();
}

SimParticle* SimTrackerHit::getSimParticle() const {
    return static_cast<SimParticle*>(simParticle_.GetObject());
}

void SimTrackerHit::setPosition(const float x, const float y, const float z) {
    this->x_ = x;
    this->y_ = y;
    this->z_ = z;
}

void SimTrackerHit::setMomentum(const float px, const float py, const float pz) {
    this->px_ = px;
    this->py_ = py;
    this->pz_ = pz;
}

}

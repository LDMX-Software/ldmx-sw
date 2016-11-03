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
    std::cout << "SimTrackerHit { " << "ID: " << std::bitset<32>(id) << ", " <<
        "Layer ID: " << layerID << ", " <<
        "Position: ( " << x << ", " << y << ", " << z << " ), " <<
        "dEdx: " << edep << ", " <<
        "Time: " << time << ", " <<
        "Momentum: ( " << px << ", " << py << ", " << pz << " )" <<
        " }" << std::endl;
}

void SimTrackerHit::Clear(Option_t*) {
    TObject::Clear();
}

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

}

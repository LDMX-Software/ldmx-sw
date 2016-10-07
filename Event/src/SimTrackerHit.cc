/**
 *
 * @file SimTrackerHit.cc
 * @brief Class used to encapsulate information from a hit in a tracking 
 *        detector.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 *
 */

#include "Event/SimTrackerHit.h"

ClassImp(SimTrackerHit)

SimTrackerHit::SimTrackerHit() 
    : TObject() {
}

SimTrackerHit::~SimTrackerHit() {
    Clear();
}

void SimTrackerHit::Print(Option_t *option) const {
    std::cout << "SimTrackerHit { " <<
            "id: " << std::bitset<32>(id) << ", " <<
            "layer id: " << layer_id << 
            "start position: ( " << start_x << ", " << start_y << ", " << start_z << " ), " <<
            "end position: ( " << end_x << ", " << end_y << ", " << end_z << " ), " <<
            "edep: " << edep << ", " <<
            "time: " << time << ", " <<
            "momentum: ( " << px << ", " << py << ", " << pz << " )" <<
            " }" << std::endl;
}

void SimTrackerHit::Clear(Option_t* /* option */) { 
    TObject::Clear(); 

}

std::vector<float> SimTrackerHit::getEndPosition() const {
    return {end_x, end_y, end_z};  
}

std::vector<float> SimTrackerHit::getStartPosition() const {
    return {start_x, start_y, start_z};  
}

std::vector<double> SimTrackerHit::getMomentum() const {
    return {px, py, pz};
}

SimParticle* SimTrackerHit::getSimParticle() const {
    return static_cast<SimParticle*>(simParticle.GetObject());
}

void SimTrackerHit::setStartPosition(const float start_x, const float start_y, const float start_z) { 
    this->start_x = start_x; 
    this->start_y = start_y; 
    this->start_z = start_z; 
}

void SimTrackerHit::setEndPosition(const float end_x, const float end_y, const float end_z) { 
    this->end_x = end_x; 
    this->end_y = end_y; 
    this->end_z = end_z; 
}

void SimTrackerHit::setMomentum(const float px, const float py, const float pz) {
    this->px = px;
    this->py = py; 
    this->pz = pz; 
}

void SimTrackerHit::setSimParticle(SimParticle* aSimParticle) {
    this->simParticle = aSimParticle;
}

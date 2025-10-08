#include "SimCore/Event/SimTrackerHit.h"

ClassImp(ldmx::SimTrackerHit);

namespace ldmx {

SimTrackerHit::~SimTrackerHit() { clear(); }

std::ostream& operator<<(std::ostream& o, const SimTrackerHit& hit) {
  return o << "SimTrackerHit { " << "id: " << hit.id_ << ", "
           << "layerID: " << hit.layer_id_ << ", "
           << "moduleID: " << hit.module_id_ << ", " << "position: ( " << hit.x_
           << ", " << hit.y_ << ", " << hit.z_ << " ), "
           << "edep: " << hit.edep_ << ", " << "time: " << hit.time_ << ", "
           << "momentum: ( " << hit.px_ << ", " << hit.py_ << ", " << hit.pz_
           << " )" << " }";
}

void SimTrackerHit::clear() {
  id_ = 0;
  layer_id_ = 0;
  module_id_ = 0;
  edep_ = 0;
  time_ = 0;
  px_ = 0;
  py_ = 0;
  pz_ = 0;
  x_ = 0;
  y_ = 0;
  z_ = 0;
  energy_ = 0;
  path_length_ = 0;
  track_id_ = -1;
  pdg_id_ = 0;
}

void SimTrackerHit::setPosition(const float x_, const float y_,
                                const float z_) {
  this->x_ = x_;
  this->y_ = y_;
  this->z_ = z_;
}

void SimTrackerHit::setMomentum(const float px, const float py,
                                const float pz) {
  this->px_ = px;
  this->py_ = py;
  this->pz_ = pz;
}
}  // namespace ldmx

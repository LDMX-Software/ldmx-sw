/**
 * @file CascadeStep.cxx
 */

#include "SimCore/Event/CascadeStep.h"

#include <cmath>

namespace ldmx {

void CascadeStep::clear() {
  history_id_ = -1;
  parent_id_ = -1;
  pdg_id_ = 0;
  px_ = py_ = pz_ = energy_ = 0;
  x_ = y_ = z_ = 0;
  generation_ = 0;
  zone_ = 0;
  daughter_ids_.clear();
  target_pdg_id_ = 0;
  interacted_ = false;
  escaped_ = false;
  stage_ = CascadeStage::UNKNOWN;
}

double CascadeStep::getKineticEnergy() const {
  double mass = getMass();
  return energy_ - mass;
}

double CascadeStep::getMass() const {
  double p2 = px_ * px_ + py_ * py_ + pz_ * pz_;
  double m2 = energy_ * energy_ - p2;
  return (m2 > 0) ? std::sqrt(m2) : 0;
}

}  // namespace ldmx


#include "Ecal/IntermediateCluster.h"

#include <iostream>

namespace ecal {

IntermediateCluster::IntermediateCluster(const ldmx::EcalHit& eh, int layer)
    : layer_{layer}, hits_{}, centroid_{} {
  add(eh);
}

void IntermediateCluster::add(const ldmx::EcalHit& eh) {
  hits_.push_back(&eh);

  double hit_e = eh.getEnergy();
  double hit_x = eh.getXPos();
  double hit_y = eh.getYPos();
  double hit_z = eh.getZPos();

  double new_e = hit_e + centroid_.E();
  centroid_.SetXYZT((centroid_.x() * centroid_.E() + hit_e * hit_x) / new_e,
                    (centroid_.y() * centroid_.E() + hit_e * hit_y) / new_e,
                    (centroid_.z() * centroid_.E() + hit_e * hit_z) / new_e,
                    new_e);
}

void IntermediateCluster::add(const ldmx::EcalHit* eh) {
  if (eh != nullptr) add(*eh);
}

void IntermediateCluster::add(const IntermediateCluster& wc) {
  double new_e = wc.centroid().E() + centroid_.E();
  centroid_.SetXYZT(
      (centroid_.x() * centroid_.E() + wc.centroid().x() * wc.centroid().E()) /
          new_e,
      (centroid_.y() * centroid_.E() + wc.centroid().y() * wc.centroid().E()) /
          new_e,
      (centroid_.z() * centroid_.E() + wc.centroid().z() * wc.centroid().E()) /
          new_e,
      new_e);

  for (const auto eh : wc.getHits()) {
    hits_.push_back(eh);
  }
}
// Set layer of the cluster centroid
void IntermediateCluster::setLayer(int layer) { layer_ = layer; }

// Get layer of the cluster centroid
int IntermediateCluster::getLayer() const { return layer_; }

}  // namespace ecal

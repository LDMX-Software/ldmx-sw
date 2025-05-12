
#include "Ecal/WorkingCluster.h"

#include <iostream>

namespace ecal {

WorkingCluster::WorkingCluster(const ldmx::EcalHit& eh, int layer)
    : layer_{layer}, hits_{}, centroid_{} {
  add(eh);
}

void WorkingCluster::add(const ldmx::EcalHit& eh) {
  hits_.push_back(&eh);

  double hitE = eh.getEnergy();
  double hitX = eh.getXPos();
  double hitY = eh.getYPos();
  double hitZ = eh.getZPos();

  double newE = hitE + centroid_.E();
  centroid_.SetXYZT((centroid_.x() * centroid_.E() + hitE * hitX) / newE,
                    (centroid_.y() * centroid_.E() + hitE * hitY) / newE,
                    (centroid_.z() * centroid_.E() + hitE * hitZ) / newE, newE);
}

void WorkingCluster::add(const ldmx::EcalHit* eh) {
  if (eh != nullptr) add(*eh);
}

void WorkingCluster::add(const WorkingCluster& wc) {
  double newE = wc.centroid().E() + centroid_.E();
  centroid_.SetXYZT(
      (centroid_.x() * centroid_.E() + wc.centroid().x() * wc.centroid().E()) /
          newE,
      (centroid_.y() * centroid_.E() + wc.centroid().y() * wc.centroid().E()) /
          newE,
      (centroid_.z() * centroid_.E() + wc.centroid().z() * wc.centroid().E()) /
          newE,
      newE);

  for (const auto eh : wc.getHits()) {
    hits_.push_back(eh);
  }
}

}  // namespace ecal

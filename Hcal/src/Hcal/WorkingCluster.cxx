
#include "Hcal/WorkingCluster.h"

#include <iostream>

namespace hcal {

WorkingCluster::WorkingCluster(const ldmx::HcalHit* eh,
                               const ldmx::HcalGeometry& hex) {
  add(eh, hex);
}

void WorkingCluster::add(const ldmx::HcalHit* eh,
                         const ldmx::HcalGeometry& hex) {
  double hit_e = eh->getEnergy();
  ROOT::Math::XYZVector hitpos = hex.getStripCenterPosition(eh->getID());
  double hit_x = hitpos.x();
  double hit_y = hitpos.y();
  double hit_z = hitpos.z();
  double hit_t = eh->getTime();
  // Based on weight for  Center-of-Gravity by hitpos*hiE/totalE
  double new_e = hit_e + centroid_.E();
  double new_centroid_x =
      (centroid_.Px() * centroid_.E() + hit_e * hit_x) / new_e;
  double new_centroid_y =
      (centroid_.Py() * centroid_.E() + hit_e * hit_y) / new_e;
  double new_centroid_z =
      (centroid_.Pz() * centroid_.E() + hit_e * hit_z) / new_e;

  if (time_ < hit_t) {
    time_ = hit_t;
  }
  centroid_.SetPxPyPzE(new_centroid_x, new_centroid_y, new_centroid_z, new_e);
  hits_.push_back(eh);
}

void WorkingCluster::add(const WorkingCluster& wc) {
  double cluster_e = wc.centroid().E();
  double centroid_x = wc.centroid().Px();
  double centroid_y = wc.centroid().Py();
  double centroid_z = wc.centroid().Pz();

  double new_e = cluster_e + centroid_.E();
  double new_centroid_x =
      (centroid_.Px() * centroid_.E() + cluster_e * centroid_x) / new_e;
  double new_centroid_y =
      (centroid_.Py() * centroid_.E() + cluster_e * centroid_y) / new_e;
  double new_centroid_z =
      (centroid_.Pz() * centroid_.E() + cluster_e * centroid_z) / new_e;

  centroid_.SetPxPyPzE(new_centroid_x, new_centroid_y, new_centroid_z, new_e);
  /*if(wc.GetTime() > time_){
      time_ = wc.GetTime();
  }*/

  std::vector<const ldmx::HcalHit*> cluster_hits = wc.getHits();

  for (unsigned int i = 0; i < cluster_hits.size(); i++) {
    hits_.push_back(cluster_hits[i]);
  }
}
}  // namespace hcal

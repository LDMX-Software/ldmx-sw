/*
   WorkingEcalCluster -- In-memory tool for working on clusters
   */

#include "Ecal/WorkingEcalCluster.h"

#include <iostream>

namespace ecal {

WorkingEcalCluster::WorkingEcalCluster(const ldmx::EcalHit& eh, int layer) {
  layer_ = layer;
  add(eh);
}

void WorkingEcalCluster::add(const ldmx::EcalHit& eh) {
  double hit_e = eh.getEnergy();

  auto hit_x = eh.getXPos();
  auto hit_y = eh.getYPos();
  auto hit_z = eh.getZPos();

  double new_e = hit_e + centroid_.E();
  double new_centroid_x = (centroid_.Px() * centroid_.E() + hit_e * hit_x) / new_e;
  double new_centroid_y = (centroid_.Py() * centroid_.E() + hit_e * hit_y) / new_e;
  double new_centroid_z = (centroid_.Pz() * centroid_.E() + hit_e * hit_z) / new_e;

  centroid_.SetPxPyPzE(new_centroid_x, new_centroid_y, new_centroid_z, new_e);

  hits_.push_back(eh);
}

void WorkingEcalCluster::add(const WorkingEcalCluster& wc) {
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

  const std::vector<ldmx::EcalHit>& cluster_hits = wc.getHits();

  for (size_t i = 0; i < cluster_hits.size(); i++) {
    hits_.push_back(cluster_hits[i]);
  }
}

}  // namespace ecal

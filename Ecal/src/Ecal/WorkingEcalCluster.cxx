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
  double hitE = eh.getEnergy();

  auto hitX = eh.getXPos();
  auto hitY = eh.getYPos();
  auto hitZ = eh.getZPos();

  double newE = hitE + centroid_.E();
  double newCentroidX = (centroid_.Px() * centroid_.E() + hitE * hitX) / newE;
  double newCentroidY = (centroid_.Py() * centroid_.E() + hitE * hitY) / newE;
  double newCentroidZ = (centroid_.Pz() * centroid_.E() + hitE * hitZ) / newE;

  centroid_.SetPxPyPzE(newCentroidX, newCentroidY, newCentroidZ, newE);

  hits_.push_back(eh);
}

void WorkingEcalCluster::add(const WorkingEcalCluster& wc) {
  double clusterE = wc.centroid().E();
  double centroidX = wc.centroid().Px();
  double centroidY = wc.centroid().Py();
  double centroidZ = wc.centroid().Pz();

  double newE = clusterE + centroid_.E();
  double newCentroidX =
      (centroid_.Px() * centroid_.E() + clusterE * centroidX) / newE;
  double newCentroidY =
      (centroid_.Py() * centroid_.E() + clusterE * centroidY) / newE;
  double newCentroidZ =
      (centroid_.Pz() * centroid_.E() + clusterE * centroidZ) / newE;

  centroid_.SetPxPyPzE(newCentroidX, newCentroidY, newCentroidZ, newE);

  const std::vector<ldmx::EcalHit>& clusterHits = wc.getHits();

  for (size_t i = 0; i < clusterHits.size(); i++) {
    hits_.push_back(clusterHits[i]);
  }
}

void WorkingEcalCluster::addMixed(const ldmx::EcalHit& eh, float percentage) {
  double hitE = eh.getEnergy()*percentage;

  auto hitX = eh.getXPos();
  auto hitY = eh.getYPos();
  auto hitZ = eh.getZPos();

  double newE = hitE + centroid_.E();
  double newCentroidX = (centroid_.Px() * centroid_.E() + hitE * hitX) / newE;
  double newCentroidY = (centroid_.Py() * centroid_.E() + hitE * hitY) / newE;
  double newCentroidZ = (centroid_.Pz() * centroid_.E() + hitE * hitZ) / newE;

  centroid_.SetPxPyPzE(newCentroidX, newCentroidY, newCentroidZ, newE);

  mixedHits_.push_back(std::make_pair(eh, percentage));
}

}  // namespace ecal

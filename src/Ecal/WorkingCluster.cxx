/*
   WorkingCluster -- In-memory tool for working on clusters
   */

#include "Ecal/WorkingCluster.h"
#include <iostream>

namespace ecal {

WorkingCluster::WorkingCluster(const ecal::event::EcalHit* eh, const ldmx::EcalHexReadout& hex) {
  add(eh, hex);
}

void WorkingCluster::add(const ecal::event::EcalHit* eh, const ldmx::EcalHexReadout& hex) {
  double hitE = eh->getEnergy();

  double hitX, hitY, hitZ;
  hex.getCellAbsolutePosition(
      eh->getID()  // this ID integer is converted into an EcalID
      ,
      hitX, hitY, hitZ);

  double newE = hitE + centroid_.E();
  double newCentroidX = (centroid_.Px() * centroid_.E() + hitE * hitX) / newE;
  double newCentroidY = (centroid_.Py() * centroid_.E() + hitE * hitY) / newE;
  double newCentroidZ = (centroid_.Pz() * centroid_.E() + hitE * hitZ) / newE;

  centroid_.SetPxPyPzE(newCentroidX, newCentroidY, newCentroidZ, newE);

  hits_.push_back(eh);
}

void WorkingCluster::add(const WorkingCluster& wc) {
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

  std::vector<const ecal::event::EcalHit*> clusterHits = wc.getHits();

  for (size_t i = 0; i < clusterHits.size(); i++) {
    hits_.push_back(clusterHits[i]);
  }
}
}  // namespace ecal

/*
   WorkingCluster -- In-memory tool for working on clusters
   */

#include "Ecal/WorkingCluster.h"
#include <iostream>

namespace ecal {

WorkingCluster::WorkingCluster(const ldmx::EcalHit* eh,
                               const ldmx::EcalGeometry& hex) {
  add(eh, hex);
}

void WorkingCluster::add(const ldmx::EcalHit* eh,
                         const ldmx::EcalGeometry& hex) {
  double hitE = eh->getEnergy();

  /// The ID number is implicitly converted to EcalID
  auto [hitX, hitY, hitZ] = hex.getPosition(eh->getID());

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

  std::vector<const ldmx::EcalHit*> clusterHits = wc.getHits();

  for (size_t i = 0; i < clusterHits.size(); i++) {
    hits_.push_back(clusterHits[i]);
  }
}
}  // namespace ecal

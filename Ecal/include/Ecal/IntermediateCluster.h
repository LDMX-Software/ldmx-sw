/*
   IntermediateCluster -- In-memory tool for working on clusters
   */
#ifndef ECAL_WORKINGCLUSTER_H_
#define ECAL_WORKINGCLUSTER_H_

#include <iostream>
#include <vector>

#include "DetDescr/EcalGeometry.h"
#include "Ecal/Event/EcalHit.h"
#include "TLorentzVector.h"

namespace ecal {

class IntermediateCluster {
 public:
  IntermediateCluster(const ldmx::EcalHit& eh, int layer_ = -1);
  IntermediateCluster() = default;
  ~IntermediateCluster() = default;
  void add(const ldmx::EcalHit& eh);
  void add(const ldmx::EcalHit* eh);
  void add(const IntermediateCluster& wc);
  const ROOT::Math::XYZTVector& centroid() const { return centroid_; }
  std::vector<const ldmx::EcalHit*> getHits() const { return hits_; }
  bool empty() const { return hits_.empty(); }
  void clear() { hits_.clear(); }
  int getLayer() const { return layer_; }

 private:
  int layer_;
  std::vector<const ldmx::EcalHit*> hits_;
  ROOT::Math::XYZTVector centroid_;
};
}  // namespace ecal

#endif

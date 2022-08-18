/*
   WorkingCluster -- In-memory tool for working on clusters
   */
#ifndef ECAL_WORKINGCLUSTER_H_
#define ECAL_WORKINGCLUSTER_H_

#include <iostream>
#include <vector>
#include "DetDescr/EcalGeometry.h"
#include "Ecal/Event/EcalHit.h"
#include "TLorentzVector.h"

namespace ecal {

class WorkingCluster {
 public:
  WorkingCluster(const ldmx::EcalHit* eh, const ldmx::EcalGeometry& geom);

  ~WorkingCluster(){};

  void add(const ldmx::EcalHit* eh, const ldmx::EcalGeometry& geom);

  void add(const WorkingCluster& wc);

  const TLorentzVector& centroid() const { return centroid_; }

  std::vector<const ldmx::EcalHit*> getHits() const { return hits_; }

  bool empty() const { return hits_.empty(); }

  void clear() { hits_.clear(); }

 private:
  std::vector<const ldmx::EcalHit*> hits_;
  TLorentzVector centroid_;
};
}  // namespace ecal

#endif

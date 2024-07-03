/*
   WorkingEcalCluster -- In-memory tool for working on clusters
   */
#ifndef ECAL_WORKINGECALCLUSTER_H_
#define ECAL_WORKINGECALCLUSTER_H_

#include <iostream>
#include <vector>

#include "DetDescr/EcalGeometry.h"
#include "Ecal/Event/EcalHit.h"
#include "TLorentzVector.h"

namespace ecal {

class WorkingEcalCluster {
 public:
  WorkingEcalCluster(const ldmx::EcalHit& eh);

  ~WorkingEcalCluster(){};

  void add(const ldmx::EcalHit& eh);

  void add(const WorkingEcalCluster& wc);

  const TLorentzVector& centroid() const { return centroid_; }

  std::vector<ldmx::EcalHit> getHits() const { return hits_; }

  bool empty() const { return hits_.empty(); }

  void clear() { hits_.clear(); }

 private:
  std::vector<ldmx::EcalHit> hits_;
  TLorentzVector centroid_;
};
}  // namespace ecal

#endif

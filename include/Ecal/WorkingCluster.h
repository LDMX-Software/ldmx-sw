/*
   WorkingCluster -- In-memory tool for working on clusters
   */
#ifndef ECAL_WORKINGCLUSTER_H_
#define ECAL_WORKINGCLUSTER_H_

#include <iostream>
#include <vector>
#include "DetDescr/EcalHexReadout.h"
#include "Ecal/Event/EcalHit.h"
#include "TLorentzVector.h"

namespace ecal {

class WorkingCluster {
 public:
  WorkingCluster(const ecal::event::EcalHit* eh, const ldmx::EcalHexReadout& geom);

  ~WorkingCluster(){};

  void add(const ecal::event::EcalHit* eh, const ldmx::EcalHexReadout& geom);

  void add(const WorkingCluster& wc);

  const TLorentzVector& centroid() const { return centroid_; }

  std::vector<const ecal::event::EcalHit*> getHits() const { return hits_; }

  bool empty() const { return hits_.empty(); }

  void clear() { hits_.clear(); }

 private:
  std::vector<const ecal::event::EcalHit*> hits_;
  TLorentzVector centroid_;
};
}  // namespace ecal

#endif

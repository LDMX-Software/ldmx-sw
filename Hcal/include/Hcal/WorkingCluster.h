
#ifndef HCAL_WORKINGCLUSTER_H_
#define HCAL_WORKINGCLUSTER_H_

#include <iostream>
#include <vector>

#include "DetDescr/HcalGeometry.h"
#include "Hcal/Event/HcalHit.h"
#include "TLorentzVector.h"
#include "TVector3.h"

namespace hcal {

class WorkingCluster {
 public:
  WorkingCluster(const ldmx::HcalHit* eh, const ldmx::HcalGeometry& geom);

  ~WorkingCluster(){};

  void add(const ldmx::HcalHit* eh, const ldmx::HcalGeometry& geom);

  void add(const WorkingCluster& wc);

  double GetTime() { return time_; }

  const TLorentzVector& centroid() const { return centroid_; }

  void SetCentroidPxPyPzE(double newCentroidX, double newCentroidY,
                          double newCentroidZ, double newE) {
    centroid_.SetPxPyPzE(newCentroidX, newCentroidY, newCentroidZ, newE);
  }

  void SetTime(double t) { time_ = t; }

  std::vector<const ldmx::HcalHit*> getHits() const { return hits_; }

  void addHit(const ldmx::HcalHit* eh) { hits_.push_back(eh); }

  bool empty() const { return hits_.empty(); }

  void clear() { hits_.clear(); }

 private:
  std::vector<const ldmx::HcalHit*> hits_;
  TLorentzVector centroid_;
  double time_ = 0;
};
}  // namespace hcal

#endif

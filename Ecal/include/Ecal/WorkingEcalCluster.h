/*
   WorkingEcalCluster -- In-memory tool for working on clusters
   */
#ifndef ECAL_WORKINGECALCLUSTER_H_
#define ECAL_WORKINGECALCLUSTER_H_

#include <iostream>
#include <vector>

#include "Ecal/Event/EcalHit.h"
#include "TLorentzVector.h"

namespace ecal {

class WorkingEcalCluster {
 public:
  WorkingEcalCluster(const ldmx::EcalHit& eh, int layer = -1);

  WorkingEcalCluster(){};

  ~WorkingEcalCluster(){};

  void add(const ldmx::EcalHit& eh);

  void add(const WorkingEcalCluster& wc);

  void addMixed(const ldmx::EcalHit& eh, double percentage);

  const TLorentzVector& centroid() const { return centroid_; }

  std::vector<ldmx::EcalHit> getHits() const { return hits_; }

  std::vector<std::pair<ldmx::EcalHit, double>> getMixedHits() const { return mixedHits_; }

  bool hasMixed() const { return mixedHits_.empty(); }

  bool empty() const { return hits_.empty() && mixedHits_.empty(); }

  void clear() { hits_.clear(); mixedHits_.clear(); }

  int getLayer() const { return layer_; }

 private:
  int layer_;
  std::vector<ldmx::EcalHit> hits_;
  std::vector<std::pair<ldmx::EcalHit, double>> mixedHits_;
  TLorentzVector centroid_;
};
}  // namespace ecal

#endif

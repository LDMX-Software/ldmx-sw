/**
 * @file HcalCluster.h
 * @brief Class that stores cluster information from the ECal
 * @author Josh Hiltbrand, University of Minnesota
 * @author Soophie Middleton, Caltech
 */

#ifndef EVENT_HCALCLUSTER_H_
#define EVENT_HCALCLUSTER_H_

// ldmx-sw
#include "Hcal/Event/HcalHit.h"
#include "Recon/Event/CaloCluster.h"

namespace ldmx {

/**
 * @class HcalCluster
 * @brief Stores cluster information from the HCal
 */
class HcalCluster : public ldmx::CaloCluster {
 public:
  /**
   * Class constructor.
   */
  HcalCluster() = default;

  /**
   * Class destructor.
   */
  virtual ~HcalCluster();

  /**
   * Reset the HcalCluster object.
   */
  void Clear();

  /**
   * Take in the hits that make up the cluster.
   * @param hit The digi hit's entry number in the events digi
   * collection.
   */
  void addHits(const std::vector<const ldmx::HcalHit*> hitsVec);

  void setTime(double x) { time_ = x; }

  double getTime() const { return time_; }

  bool operator<(const HcalCluster& rhs) const {
    return this->getEnergy() < rhs.getEnergy();
  }

 private:
  double time_{0};

  ClassDef(HcalCluster, 1);
};
}  // namespace ldmx

#endif

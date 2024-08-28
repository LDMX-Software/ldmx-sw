/**
 * @file EcalCluster.h
 * @brief Class that stores cluster information from the ECal
 * @author Josh Hiltbrand, University of Minnesota
 */

#ifndef EVENT_ECALCLUSTER_H_
#define EVENT_ECALCLUSTER_H_

// ldmx-sw
#include "Ecal/Event/EcalHit.h"
#include "Recon/Event/CaloCluster.h"

namespace ldmx {

/**
 * @class EcalCluster
 * @brief Stores cluster information from the ECal
 */
class EcalCluster : public ldmx::CaloCluster {
 public:
  /**
   * Class constructor.
   */
  EcalCluster();

  /**
   * Class destructor.
   */
  virtual ~EcalCluster();

  /**
   * Take in the hits that make up the cluster.
   * @param hit The digi hit's entry number in the events digi
   * collection.
   */
  void addHits(const std::vector<const ldmx::EcalHit*> hitsVec);

  void addHits(const std::vector<ldmx::EcalHit> hitsVec);

  void addFirstLayerHits(const std::vector<ldmx::EcalHit> hitsVec);

  bool operator<(const EcalCluster& rhs) const {
    return this->getEnergy() < rhs.getEnergy();
  }

  void setFirstLayerCentroidXYZ(double x, double y, double z) {
    firstLayerCentroidX_ = x;
    firstLayerCentroidY_ = y;
    firstLayerCentroidZ_ = z;
  }

  double getFirstLayerCentroidX() const { return firstLayerCentroidX_; }
  double getFirstLayerCentroidY() const { return firstLayerCentroidY_; }
  double getFirstLayerCentroidZ() const { return firstLayerCentroidZ_; }

  std::vector<unsigned int> getFirstLayerHitIDs() const {
    return firstLayerHitIDs_;
  }

 private:
  // Could add further ECal-specific info here...

  std::vector<unsigned int> firstLayerHitIDs_;

  double firstLayerCentroidX_{0};
  double firstLayerCentroidY_{0};
  double firstLayerCentroidZ_{0};

  ClassDef(EcalCluster, 1);
};
}  // namespace ldmx

#endif

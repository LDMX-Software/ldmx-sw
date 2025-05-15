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
  void addHits(const std::vector<const ldmx::EcalHit*>& hits);

  void addFirstLayerHits(const std::vector<const ldmx::EcalHit*>& hits);

  bool operator<(const EcalCluster& rhs) const {
    return this->getEnergy() < rhs.getEnergy();
  }

  void setFirstLayerCentroidXYZ(double x, double y, double z) {
    first_layer_centroid_x_ = x;
    first_layer_centroid_y_ = y;
    first_layer_centroid_z_ = z;
  }

  double getFirstLayerCentroidX() const { return first_layer_centroid_x_; }
  double getFirstLayerCentroidY() const { return first_layer_centroid_y_; }
  double getFirstLayerCentroidZ() const { return first_layer_centroid_z_; }

  std::vector<unsigned int> getFirstLayerHitIDs() const {
    return first_layer_hit_IDs_;
  }

 private:
  // Could add further ECal-specific info here...

  std::vector<unsigned int> first_layer_hit_IDs_;

  double first_layer_centroid_x_{0};
  double first_layer_centroid_y_{0};
  double first_layer_centroid_z_{0};

  ClassDef(EcalCluster, 1);
};
}  // namespace ldmx

#endif

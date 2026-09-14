/**
 * @file DBScanClusterBuilder.h
 * @brief Implementation of DBSCAN clustering algo
 * @author Christian Herwig, Fermilab
 */

#ifndef DBSCANCLUSTERBUILDER_H
#define DBSCANCLUSTERBUILDER_H

#include "Framework/EventProcessor.h"
#include "Recon/Event/CaloCluster.h"
#include "Recon/Event/CalorimeterHit.h"
#include "TFitResult.h"
#include "TGraph.h"
namespace recon {

/**
 * @class DBScanClusterBuilder
 * @brief
 */
class DBScanClusterBuilder {
 public:
  DBScanClusterBuilder();

  DBScanClusterBuilder(float minHitEnergy, float clusterHitDist,
                       float clusterZBias,
                       float minClusterHitMult);  // overloaded constructor

  std::vector<std::vector<const ldmx::CalorimeterHit*> > runDBSCAN(
      const std::vector<const ldmx::CalorimeterHit*>& hits_);

  void fillClusterInfoFromHits(ldmx::CaloCluster* cl,
                               std::vector<const ldmx::CalorimeterHit*> hits_,
                               bool logEnergyWeight,
                               bool saveHitContribs = true);

  void setMinHitEnergy(float x_) { min_hit_energy_ = x_; }

  void setMinHitDistance(float x_) { cluster_hit_dist_ = x_; }

  void setZBias(float x_) {
    cluster_z_bias_ = x_;
  }  // set the z_ bias of the cluster

  void setMinHitMultiplicity(int x_) { min_cluster_hit_mult_ = x_; }

  float getMinHitEnergy() const { return min_hit_energy_; };

  float setMinHitDistance() const { return cluster_hit_dist_; }

  int setMinHitMultiplicity() const { return min_cluster_hit_mult_; }

 private:
  bool isIn(unsigned int i, const std::vector<unsigned int>& l) {
    return std::find(l.begin(), l.end(), i) != l.end();
  }
  float dist(const ldmx::CalorimeterHit* a, const ldmx::CalorimeterHit* b) {
    return sqrt(pow(a->getXPos() - b->getXPos(), 2)  // distance
                + pow(a->getYPos() - b->getYPos(), 2) +
                pow((a->getZPos() - b->getZPos()) / cluster_z_bias_,
                    2));  // divide by the z_ bias
  }

  float min_hit_energy_{0};
  float cluster_hit_dist_{100.};
  float cluster_z_bias_{1.};  // private parameter for z_ bias
  int min_cluster_hit_mult_{2};
  /// Enable logging
  enableLogging("DBScanClusterBuilder")
};
}  // namespace recon

#endif /* DBSCANCLUSTERBUILDER_H */

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

  bool operator<(const EcalCluster& rhs) const {
    return this->getEnergy() < rhs.getEnergy();
  }

 private:
  // Could add further ECal-specific info here...
  
  ClassDef(EcalCluster, 1);
};
}  // namespace ldmx

#endif

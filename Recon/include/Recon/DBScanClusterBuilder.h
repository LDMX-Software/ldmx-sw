/**
 * @file DBScanClusterBuilder.h
 * @brief Implementation of DBSCAN clustering algo
 * @author Christian Herwig, Fermilab
 */

#ifndef DBSCANCLUSTERBUILDER_H
#define DBSCANCLUSTERBUILDER_H

#include "Recon/Event/CalorimeterHit.h"
#include "Recon/Event/CaloCluster.h"
#include "TGraph.h"
#include "TFitResult.h"

namespace recon {

/**
 * @class DBScanClusterBuilder
 * @brief
 */
class DBScanClusterBuilder {
 public:

  DBScanClusterBuilder();

  DBScanClusterBuilder(float minHitEnergy, float clusterHitDist, float clusterZBias, float minClusterHitMult);  // overloaded constructor

  std::vector<std::vector<const ldmx::CalorimeterHit*> > runDBSCAN( 
    const std::vector<const ldmx::CalorimeterHit*> &hits, bool debug);

  void fillClusterInfoFromHits(ldmx::CaloCluster *cl, 
			       std::vector<const ldmx::CalorimeterHit*> hits,
			       bool logEnergyWeight);

  void setMinHitEnergy(float x){minHitEnergy_=x;}

  void setMinHitDistance(float x){clusterHitDist_=x;}

  void setZBias(float x){clusterZBias_=x;} // set the z bias of the cluster

  void setMinHitMultiplicity(int x){minClusterHitMult_=x;}

  float getMinHitEnergy() const { return minHitEnergy_;};

  float setMinHitDistance() const {return clusterHitDist_;}

  int setMinHitMultiplicity() const {return minClusterHitMult_;}

 private:
  bool isIn(unsigned int i, std::vector<unsigned int> l){
    return std::find(l.begin(), l.end(), i) != l.end();
  }
  float dist(const ldmx::CalorimeterHit * a, const ldmx::CalorimeterHit * b){
    return sqrt( pow(a->getXPos() - b->getXPos(),2)  // distance
               + pow(a->getYPos() - b->getYPos(),2)
	       + pow((a->getZPos() - b->getZPos())/clusterZBias_,2) );  // divide by the z bias
  }
  
  float minHitEnergy_{0};
  float clusterHitDist_{100.};
  float clusterZBias_{1.};  // private parameter for z bias
  int minClusterHitMult_{2};

};
}  // namespace recon


#endif /* DBSCANCLUSTERBUILDER_H */

/**
 * @file DBScanClusterBuilder.h
 * @brief Implementation of DBSCAN clustering algo
 * @author Christian Herwig, Fermilab
 */

#ifndef DBSCANCLUSTERBUILDER_H
#define DBSCANCLUSTERBUILDER_H

// LDMX Framework
/* #include "Framework/Configure/Parameters.h"  // Needed to import parameters from configuration file */
/* #include "Framework/Event.h" */
/* #include "Framework/EventProcessor.h"  //Needed to declare processor */

/* #include "DetDescr/EcalGeometry.h" */
/* #include "Ecal/Event/EcalHit.h" */
/* #include "Ecal/Event/EcalCluster.h" */
/* #include "Hcal/Event/HcalHit.h" */
/* #include "Hcal/Event/HcalCluster.h" */
#include "Recon/Event/CalorimeterHit.h"
/* #include "Ecal/MyClusterWeight.h" */
/* #include "Ecal/TemplatedClusterFinder.h" */
/* #include "Ecal/WorkingCluster.h" */
#include "TGraph.h"
#include "TFitResult.h"

/* using std::cout; */
/* using std::endl; */

namespace recon {

/**
 * @class DBScanClusterBuilder
 * @brief
 */
class DBScanClusterBuilder {
 public:

  DBScanClusterBuilder();

  DBScanClusterBuilder(float minHitEnergy, float clusterHitDist, float minClusterHitMult);

  std::vector<std::vector<const ldmx::CalorimeterHit*> > runDBSCAN( 
    const std::vector<const ldmx::CalorimeterHit*> &hits, bool debug);

  template <class C>
  void fillClusterInfoFromHits(C &cl, std::vector<const ldmx::CalorimeterHit*> hits,
				 float minHitEnergy, bool logEnergyWeight);

  void setMinHitEnergy(float x){minHitEnergy_=x;}

  void setMinHitDistance(float x){clusterHitDist_=x;}

  void setMinHitMultiplicity(int x){minClusterHitMult_=x;}

  float getMinHitEnergy() const { return minHitEnergy_;};

  float setMinHitDistance() const {return clusterHitDist_;}

  int setMinHitMultiplicity() const {return minClusterHitMult_;}

 private:
  bool isIn(unsigned int i, std::vector<unsigned int> l){
    return std::find(l.begin(), l.end(), i) != l.end();
  }
  float dist (const ldmx::CalorimeterHit* a, const ldmx::CalorimeterHit* b){
    return sqrt( pow(a->getXPos() - b->getXPos(),2)
		 + pow(a->getYPos() - b->getYPos(),2)
		 + pow(a->getZPos() - b->getZPos(),2));
  }
  
  // specific verbosity of this producer
  /* int verbose_{0}; */
  /* bool singleCluster_{true}; */
  /* bool logEnergyWeight_{true}; */

  float minHitEnergy_{0};
  float clusterHitDist_{100.};
  int minClusterHitMult_{2};

  // name of collection for hits to be passed as input
  /* std::string hitCollName_; */
  /* // name of collection for pfCluster to be output */
  /* std::string clusterCollName_; */
  /* std::string suffix_; */
};
}  // namespace recon

/* typedef DBScanClusterBuilder<ECalCluster, ECalHit> EcalDBScanClusterBuilder; */
/* typedef DBScanClusterBuilder<HcalCluster, HcalHit> HcalDBScanClusterBuilder; */

#endif /* DBSCANCLUSTERBUILDER_H */

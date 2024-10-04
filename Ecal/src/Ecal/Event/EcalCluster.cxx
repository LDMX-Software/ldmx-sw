#include "Ecal/Event/EcalCluster.h"

ClassImp(ldmx::EcalCluster)

    namespace ldmx {
  EcalCluster::EcalCluster() {}

  EcalCluster::~EcalCluster() { Clear(); }

  void EcalCluster::addHits(const std::vector<const EcalHit *> hitsVec) {
    std::vector<unsigned int> vecIDs;
    for (int iHit = 0; iHit < hitsVec.size(); iHit++) {
      vecIDs.push_back(hitsVec[iHit]->getID());
    }
    setIDs(vecIDs);
  }

  void EcalCluster::addHits(const std::vector<EcalHit> hitsVec) {
    std::vector<unsigned int> vecIDs;
    for (int iHit = 0; iHit < hitsVec.size(); iHit++) {
      vecIDs.push_back(hitsVec[iHit].getID());
    }
    setIDs(vecIDs);
  }

  void EcalCluster::addFirstLayerHits(const std::vector<EcalHit> hitsVec) {
    std::vector<unsigned int> vecIDs;
    for (int iHit = 0; iHit < hitsVec.size(); iHit++) {
      vecIDs.push_back(hitsVec[iHit].getID());
    }
    firstLayerHitIDs_ = vecIDs;
  }
}  // namespace ldmx

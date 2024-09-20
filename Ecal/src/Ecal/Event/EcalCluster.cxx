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

  void EcalCluster::findHitOrigins(const std::vector<ldmx::SimCalorimeterHit>& ecalSimHits) {
    
    std::vector<unsigned int> vecIDs;
    for (const auto& id : this->getHitIDs()) {
      int tag = 0;
      auto it = std::find_if(ecalSimHits.begin(), ecalSimHits.end(),
        [&](const auto& simHit) { return simHit.getID() == id; });
      if (it != ecalSimHits.end()) {
        int ancestor = 0;
        int prevAncestor = 0;
        bool tagged = false;
        tag = 0;
        for (int i = 0; i < it->getNumberOfContribs(); i++) {
          // for each contrib in this simhit
          const auto& c = it->getContrib(i);
          // get origin electron ID
          ancestor = c.originID;
          if (!tagged && i != 0 && prevAncestor != ancestor) {
            // if origin electron ID does not match previous origin electron ID
            // this hit has contributions from several electrons, ie mixed case
            tag = 0;
            tagged = true;
          }
          prevAncestor = ancestor;
        }
        if (!tagged) {
          // if not tagged, hit was from a single electron
          tag = prevAncestor;
        }
      }
      else {tag = -1;}
      vecIDs.push_back(tag);
    }
    hitOriginIDs_ = vecIDs;           
  }

}  // namespace ldmx

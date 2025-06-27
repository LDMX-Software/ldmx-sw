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
    std::vector<unsigned int> vecIDs_incident;
    for (const auto& id : this->getHitIDs()) {
      int tag = 0;
      int tag_incident = 0;
      auto it = std::find_if(ecalSimHits.begin(), ecalSimHits.end(),
        [&](const auto& simHit) { return simHit.getID() == id; });
      if (it != ecalSimHits.end()) {
        int ancestor = 0;
        int prevAncestor = 0;
        bool tagged = false;
        int ancestor_incident = 0;
        int prevAncestor_incident = 0;
        bool tagged_incident = false;
        tag = 0;
        tag_incident = 0;
        for (int i = 0; i < it->getNumberOfContribs(); i++) {
          // for each contrib in this simhit
          const auto& c = it->getContrib(i);
          // get origin electron ID
          ancestor = c.originID;
          ancestor_incident = c.incidentID;

          if (!tagged && i != 0 && prevAncestor != ancestor) {
            // if origin electron ID does not match previous origin electron ID
            // this hit has contributions from several electrons, ie mixed case
            tag = 0;
            tagged = true;
          }
          if (!tagged_incident && i != 0 && prevAncestor_incident != ancestor_incident) {
            // if origin electron ID does not match previous origin electron ID
            // this hit has contributions from several electrons, ie mixed case
            tag_incident= 0;
            tagged_incident = true;
          }
          prevAncestor = ancestor;
          prevAncestor_incident = ancestor_incident;
        }
        if (!tagged) {
          // if not tagged, hit was from a single electron
          tag = prevAncestor;
        }
        if (!tagged_incident) {
          // if not tagged, hit was from a single electron
          tag_incident = prevAncestor_incident;
        }
      }
      else {
        tag = -1;
        tag_incident = -1;
      }
      vecIDs.push_back(tag);
      vecIDs_incident.push_back(tag_incident);
    }
    hitOriginIDs_ = vecIDs;
    hitIncidentIDs_ = vecIDs_incident;           
  }

}  // namespace ldmx

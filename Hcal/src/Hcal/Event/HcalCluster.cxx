#include "Hcal/Event/HcalCluster.h"

ClassImp(ldmx::HcalCluster);

namespace ldmx {
HcalCluster::~HcalCluster() { Clear(); }

void HcalCluster::Clear() {
  ldmx::CaloCluster::Clear();
  time_ = 0;
}

void HcalCluster::addHits(const std::vector<const HcalHit *> hitsVec) {
  std::vector<unsigned int> vecIDs;
  for (unsigned int iHit = 0; iHit < hitsVec.size(); iHit++) {
    vecIDs.push_back(hitsVec[iHit]->getID());
  }
  setIDs(vecIDs);
}
}  // namespace ldmx

#include "Hcal/Event/HcalCluster.h"

ClassImp(ldmx::HcalCluster);

namespace ldmx {
HcalCluster::~HcalCluster() { clear(); }

void HcalCluster::clear() {
  ldmx::CaloCluster::clear();
  time_ = 0;
}

void HcalCluster::addHits(const std::vector<const HcalHit *> hitsVec) {
  std::vector<unsigned int> vec_i_ds;
  for (unsigned int i_hit = 0; i_hit < hitsVec.size(); i_hit++) {
    vec_i_ds.push_back(hitsVec[i_hit]->getID());
  }
  setIDs(vec_i_ds);
}
}  // namespace ldmx

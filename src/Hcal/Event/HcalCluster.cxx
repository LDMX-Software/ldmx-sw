#include "Hcal/Event/HcalCluster.h"

ClassImp(ldmx::HcalCluster);

namespace ldmx {
HcalCluster::~HcalCluster() { Clear(); }

// void HcalCluster::Print() const {
//   std::cout << "HcalCluster { "
//             << "Energy: " << energy_ << ", "
//             << "Number of hits: " << nHits_ << " }" << std::endl;
// }

void HcalCluster::Clear() {
  this->Clear();
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

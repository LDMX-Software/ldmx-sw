#include "Hcal/Event/HcalCluster.h"

ClassImp(ldmx::HcalCluster)

    namespace ldmx {
  HcalCluster::HcalCluster() {}

  HcalCluster::~HcalCluster() { Clear(); }

  void HcalCluster::Print() const {
    std::cout << "HcalCluster { "
              << "Energy: " << energy_ << ", "
              << "Number of hits: " << nHits_ << " }" << std::endl;
  }

  void HcalCluster::Clear() {
    hitIDs_.clear();

    energy_ = 0;
    nHits_ = 0;
    centroidX_ = 0;
    centroidY_ = 0;
    centroidZ_ = 0;
  }

  void HcalCluster::addHits(const std::vector<const HcalHit *> hitsVec) {

    std::vector<unsigned int> vecIDs;
    for (int iHit = 0; iHit < hitsVec.size(); iHit++) {
      vecIDs.push_back(hitsVec[iHit]->getID());
    }

    setIDs(vecIDs);
  }
}  // namespace ldmx

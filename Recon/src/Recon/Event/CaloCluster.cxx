#include "Recon/Event/CaloCluster.h"

ClassImp(ldmx::CaloCluster);

namespace ldmx {

CaloCluster::~CaloCluster() { Clear(); }

std::ostream& operator<<(std::ostream& o, const CaloCluster& c) {
  return o << "CaloCluster { " << "Energy: " << c.energy_ << ", "
           << "Number of hits: " << c.nHits_ << " }";
}

void CaloCluster::Clear() {
  hitIDs_.clear();
  energy_ = 0;
  nHits_ = 0;
  centroidX_ = 0;
  centroidY_ = 0;
  centroidZ_ = 0;
  rmsX_ = 0;
  rmsY_ = 0;
  rmsZ_ = 0;
  DXDZ_ = 0;
  DYDZ_ = 0;
  errDXDZ_ = 0;
  errDYDZ_ = 0;
}

void CaloCluster::addHits(const std::vector<const CalorimeterHit*> hitsVec) {
  std::vector<unsigned int> vecIDs;
  for (unsigned int iHit = 0; iHit < hitsVec.size(); iHit++) {
    vecIDs.push_back(hitsVec[iHit]->getID());
  }
  setIDs(vecIDs);
}

}  // namespace ldmx

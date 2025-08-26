#include "Recon/Event/CaloCluster.h"

ClassImp(ldmx::CaloCluster);

namespace ldmx {

CaloCluster::~CaloCluster() { clear(); }

std::ostream& operator<<(std::ostream& o, const CaloCluster& c) {
  return o << "CaloCluster { " << "Energy: " << c.energy_ << ", "
           << "Number of hits_: " << c.n_hits_ << " }";
}

void CaloCluster::clear() {
  hit_ids.clear();
  energy_ = 0;
  n_hits_ = 0;
  centroid_x_ = 0;
  centroid_y_ = 0;
  centroid_z_ = 0;
  rms_x_ = 0;
  rms_y_ = 0;
  rms_z_ = 0;
  dxdz_ = 0;
  dydz_ = 0;
  err_dxdz_ = 0;
  err_dydz_ = 0;
}

void CaloCluster::addHits(const std::vector<const CalorimeterHit*> hitsVec) {
  std::vector<unsigned int> vec_i_ds;
  for (unsigned int i_hit = 0; i_hit < hitsVec.size(); i_hit++) {
    vec_i_ds.push_back(hitsVec[i_hit]->getID());
  }
  setIDs(vec_i_ds);
}

}  // namespace ldmx

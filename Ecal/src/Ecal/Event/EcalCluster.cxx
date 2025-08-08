#include "Ecal/Event/EcalCluster.h"

ClassImp(ldmx::EcalCluster);

namespace ldmx {

EcalCluster::~EcalCluster() { clear(); }

void EcalCluster::addHits(const std::vector<const EcalHit*>& hits_) {
  std::vector<unsigned int> ids;
  ids.reserve(hits_.size());
  for (const auto& h : hits_) {
    ids.push_back(h->getID());
  }
  setIDs(ids);
}

void EcalCluster::addFirstLayerHits(const std::vector<const EcalHit*>& hits_) {
  first_layer_hit_ids_.clear();
  first_layer_hit_ids_.reserve(hits_.size());
  for (const auto& h : hits_) {
    first_layer_hit_ids_.push_back(h->getID());
  }
}

}  // namespace ldmx

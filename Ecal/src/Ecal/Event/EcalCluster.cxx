#include "Ecal/Event/EcalCluster.h"

ClassImp(ldmx::EcalCluster);

namespace ldmx {

EcalCluster::~EcalCluster() { clear(); }

void EcalCluster::addHits(const std::vector<const EcalHit*>& hits) {
  std::vector<unsigned int> ids;
  ids.reserve(hits.size());
  for (const auto& h : hits) {
    ids.push_back(h->getID());
  }
  setIDs(ids);
}

void EcalCluster::addFirstLayerHits(const std::vector<const EcalHit*>& hits) {
  first_layer_hit_ids_.clear();
  first_layer_hit_ids_.reserve(hits.size());
  for (const auto& h : hits) {
    first_layer_hit_ids_.push_back(h->getID());
  }
}

}  // namespace ldmx

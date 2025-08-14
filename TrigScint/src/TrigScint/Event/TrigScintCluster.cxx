#include "TrigScint/Event/TrigScintCluster.h"

ClassImp(ldmx::TrigScintCluster);

namespace ldmx {
TrigScintCluster::~TrigScintCluster() { clear(); }

std::ostream& operator<<(std::ostream& o, const TrigScintCluster& c) {
  o << "TrigScintCluster { " << "Energy: " << c.energy_ << ", "
    << "Number of hits: " << c.n_hits_ << ", " << "Seed channel " << c.seed_
    << ", Channel centroid: " << c.centroid_ << " }";
  o << "  --  Constituent hit channel ids: {  ";
  for (const auto& idx : c.getHitIDs()) {
    o << idx << "  ";

    o << "}";
  }
  return o;
};

void TrigScintCluster::clear(Option_t*) {
  hit_ids_.clear();

  centroid_x_ = 0;
  centroid_y_ = 0;
  centroid_z_ = 0;
  setEnergy(0);
  setNHits(0);
  setCentroid(0);
  setSeed(-1);
}
}  // namespace ldmx

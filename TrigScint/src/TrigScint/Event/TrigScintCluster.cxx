#include "TrigScint/Event/TrigScintCluster.h"

ClassImp(ldmx::TrigScintCluster);

namespace ldmx {
TrigScintCluster::~TrigScintCluster() { Clear(); }

std::ostream& operator<<(std::ostream& o, const TrigScintCluster& c) {
  o << "TrigScintCluster { " << "Energy: " << c.energy_ << ", "
    << "Number of hits: " << c.nHits_ << ", " << "Seed channel " << c.seed_
    << ", Channel centroid: " << c.centroid_ << " }";
  o << "  --  Constituent hit channel ids: {  ";
  for (const auto& idx : c.getHitIDs()) {
    o << idx << "  ";

    o << "}";
  }
  return o;
};

void TrigScintCluster::Clear(Option_t*) {
  hitIDs_.clear();

  centroidX_ = 0;
  centroidY_ = 0;
  centroidZ_ = 0;
  setEnergy(0);
  setNHits(0);
  setCentroid(0);
  setSeed(-1);
}
}  // namespace ldmx

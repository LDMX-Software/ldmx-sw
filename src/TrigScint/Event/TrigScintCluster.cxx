#include "TrigScint/Event/TrigScintCluster.h"

ClassImp(ldmx::TrigScintCluster)

namespace ldmx {

TrigScintCluster::~TrigScintCluster() { Clear(); }

void TrigScintCluster::Print(Option_t *option) const {

  std::cout << "TrigScintCluster { "
            << "Energy: " << energy_ << ", "
            << "Number of hits: " << nHits_ << ", "
            << "Seed channel " << seed_ << ", Channel centroid: " << centroid_
            << " }" << std::endl;
  std::cout << "  --  Constituent hit channel ids: {  ";
  for (const auto &idx : getHitIDs())
    std::cout << idx << "  ";
  std::cout << "}" << std::endl;
}

void TrigScintCluster::Clear(Option_t *) {

  hitIDs_.clear();

  centroidX_ = 0;
  centroidY_ = 0;
  centroidZ_ = 0;
  setEnergy(0);
  setNHits(0);
  setCentroid(0);
  setSeed(-1);
}
} // namespace ldmx

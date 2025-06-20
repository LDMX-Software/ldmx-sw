#include "Ecal/Event/EcalMipCollection.h"

ClassImp(ldmx::EcalMipCollection);

namespace ldmx {

void EcalMipCollection::Print() const {
std::cout << "[ EcalMipCollection ]:\n"
        << "\t ele_trajectory size : " << ele_trajectory_.size() << "\n"
        << "\t photon_trajectory size : " << photon_trajectory_.size() << "\n"
        << "\t tracking_hit_list size : " << tracking_hit_list_.size() << "\n"
        << std::endl;
}

void EcalMipCollection::Clear() {
  ele_trajectory_.clear();
  photon_trajectory_.clear();
  tracking_hit_list_.clear();

//   nStraightTracks_ = 0;
//   nLinregTracks_ = 0;
//   firstNearPhLayer_ = 0;
//   nNearPhHits_ = 0;
//   epAng_ = 0;
//   epSep_ = 0;
//   epDot_ = 0;
//   photonTerritoryHits_ = 0;

  // Reset profiling map
//   profiling_map_.clear();
}

}  // namespace ldmx

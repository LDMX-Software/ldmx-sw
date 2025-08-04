#include "Ecal/Event/EcalTrajectoryInfo.h"

ClassImp(ldmx::EcalTrajectoryInfo);

namespace ldmx {

std::ostream& operator<<(std::ostream& o, const EcalTrajectoryInfo& c) {
  return o << "EcalTrajectoryInfo { ele_trajectory size : " << c.ele_trajectory_.size() << ", "
           << "photon_trajectory size : " << c.photon_trajectory_.size() << ", "
           << "tracking_hit_list size : " << c.tracking_hit_list_.size() << " }";
}

void EcalTrajectoryInfo::Clear() {
  ele_trajectory_.clear();
  photon_trajectory_.clear();
  tracking_hit_list_.clear();
}

}  // namespace ldmx

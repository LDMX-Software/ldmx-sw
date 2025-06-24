#include "Ecal/Event/EcalTrajectoryInfo.h"

ClassImp(ldmx::EcalTrajectoryInfo);

namespace ldmx {

void EcalTrajectoryInfo::Print() const {
std::cout << "[ EcalTrajectoryInfo ]:\n"
        << "\t ele_trajectory size : " << ele_trajectory_.size() << "\n"
        << "\t photon_trajectory size : " << photon_trajectory_.size() << "\n"
        << "\t tracking_hit_list size : " << tracking_hit_list_.size() << "\n"
        << std::endl;
}

void EcalTrajectoryInfo::Clear() {
  ele_trajectory_.clear();
  photon_trajectory_.clear();
  tracking_hit_list_.clear();
}

}  // namespace ldmx

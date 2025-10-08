#include "Recon/Event/FiducialFlag.h"

ClassImp(ldmx::FiducialFlag);

namespace ldmx {

std::ostream& operator<<(std::ostream& o, const FiducialFlag& c) {
  return o << "FiducialFlag { " << "fiducialFlag: " << c.fiducial_flag_ << ", "
           << "isFiducial: " << c.is_fiducial_ << ", "
           << "hasEcalHit: " << c.has_ecal_hit_ << ", "
           << "hasHcalHit: " << c.has_hcal_hit_ << ", "
           << "hasMinTrackerHits: " << c.has_min_tracker_hits_ << ", "
           << "hasMinEnergy: " << c.has_min_energy_ << " }";

  for (int i = 0; i < c.variables_.GetSize(); ++i) {
    std::cout << "Element " << i << " : " << c.variables_[i] << std::endl;
  }
}

void FiducialFlag::clear() {
  fiducial_flag_ = 0;
  is_fiducial_ = false;
  has_ecal_hit_ = false;
  has_hcal_hit_ = false;
  has_min_tracker_hits_ = false;
  has_min_energy_ = false;

  for (int i = 0; i < variables_.GetSize(); ++i) {
    variables_[i] = 0;
  }
}

void FiducialFlag::setFiducialFlag(int fiducial_flag, int nvar) {
  fiducial_flag_ = fiducial_flag;

  if (nvar > variables_.GetSize()) {
    variables_.Set(nvar);
  }
}

void FiducialFlag::setAlgoVar(int element, double value) {
  if (element >= 0 && element < variables_.GetSize()) {
    variables_[element] = value;
  }
}
}  // namespace ldmx

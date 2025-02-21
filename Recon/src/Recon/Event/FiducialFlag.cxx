#include "Recon/Event/FiducialFlag.h"

ClassImp(ldmx::FiducialFlag)

    namespace ldmx {
  FiducialFlag::FiducialFlag() {}

  void FiducialFlag::Print() const {
    std::cout << "FiducialFlag { "
              << "fiducialFlag: " << fiducial_flag_ << ", "
              << "isFiducial: " << is_fiducial_ << ", "
              << "hasEcalHit: " << has_ecal_hit_ << ", "
              << "hasHcalHit: " << has_hcal_hit_ << ", "
              << "hasMinTrackerHits: " << has_min_tracker_hits_ << ", "
              << "hasMinEnergy: " << has_min_energy_ << " }" << std::endl;

    for (int i = 0; i < variables_.GetSize(); ++i) {
      std::cout << "Element " << i << " : " << variables_[i] << std::endl;
    }
  }

  void FiducialFlag::Clear() {
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

  void FiducialFlag::setIsFiducial(bool is_fiducial) {
    is_fiducial_ = is_fiducial;
  }

  void FiducialFlag::setHasEcalHit(bool has_ecal_hit) {
    has_ecal_hit_ = has_ecal_hit;
  }

  void FiducialFlag::setHasHcalHit(bool has_hcal_hit) {
    has_hcal_hit_ = has_hcal_hit;
  }

  void FiducialFlag::setHasMinTrackerHits(bool has_min_tracker_hits) {
    has_min_tracker_hits_ = has_min_tracker_hits;
  }

  void FiducialFlag::setHasMinEnergy(bool has_min_energy) {
    has_min_energy_ = has_min_energy;
  }

  void FiducialFlag::setAlgoVar(int element, double value) {
    if (element >= 0 && element < variables_.GetSize()) {
      variables_[element] = value;
    }
  }
}  // namespace ldmx

#include "Recon/Event/FiducialFlag.h"

ClassImp(ldmx::FiducialFlag)

    namespace ldmx {
   FiducialFlag::FiducialFlag() {}

   FiducialFlag::~FiducialFlag() { Clear(); }

  void FiducialFlag::Print() const {
    std::cout << "FiducialFlag { "
              << "fiducialFlag: " << fiducialFlag_ << ", "
              << "isFiducial: " << isFiducial_ << ", "
              << "hasEcalHit: " << hasEcalHit_ << ", "
              << "hasHcalHit: " << hasHcalHit_ << ", "
              << "hasMinTrackerHits: " << hasMinTrackerHits_ << ", "
              << "hasMinEnergy: " << hasMinEnergy_ << " }" << std::endl;

    for (int i = 0; i < variables_.GetSize(); ++i) {
      std::cout << "Element " << i << " : " << variables_[i] << std::endl;
    }
  }

  void FiducialFlag::Clear() {
    fiducialFlag_ = 0;
    isFiducial_ = false;
    hasEcalHit_ = false;
    hasHcalHit_ = false;
    hasMinTrackerHits_ = false;
    hasMinEnergy_ = false;

    for (int i = 0; i < variables_.GetSize(); ++i) {
      variables_[i] = 0;
    }
  }

  void FiducialFlag::setFiducialFlag(int fiducialFlag, int nvar) {
    fiducialFlag_ = fiducialFlag;

    if (nvar > variables_.GetSize()) {
      variables_.Set(nvar);
    }
  }

  void FiducialFlag::setIsFiducial(bool isFiducial) {isFiducial_ = isFiducial;}

  void FiducialFlag::setHasEcalHit(bool hasEcalHit) {hasEcalHit_ = hasEcalHit;}

  void FiducialFlag::setHasHcalHit(bool hasHcalHit) {hasHcalHit_ = hasHcalHit;}

  void FiducialFlag::setHasMinTrackerHits(bool hasMinTrackerHits) {hasMinTrackerHits_ = hasMinTrackerHits;}

  void FiducialFlag::setHasMinEnergy(bool hasMinEnergy) {hasMinEnergy_ = hasMinEnergy;}

  void FiducialFlag::setAlgoVar(int element, double value) {
    if (element >= 0 && element < variables_.GetSize()) {
      variables_[element] = value;
    }
  }
}  // namespace ldmx

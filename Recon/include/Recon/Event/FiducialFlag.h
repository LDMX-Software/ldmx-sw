/**
 * @file FiducialFlag.h
 * @brief Class that holds truth-level fiduciality flags on
 * the signal recoil electron.
 * @author Elizabeth Berzin, Stanford University
 */

#ifndef RECON_EVENT_FIDUCIALFLAG_H_
#define RECON_EVENT_FIDUCIALFLAG_H_

// ROOT
#include "TArrayD.h"
#include "TObject.h"  //ClassDef
#include "TString.h"

// STL
#include <iostream>

namespace ldmx {

/**
 * @class FiducialFlag
 * @brief Holds truth-level fiduciality flags on the signal recoil electron
 */
class FiducialFlag {
 public:
  /**
   * Class constructor.
   */
  FiducialFlag() = default;

  /**
   * Class destructor.
   */
  virtual ~FiducialFlag() = default;

  /**
   * Print a description of this object.
   */
  friend std::ostream &operator<<(std::ostream &o, const FiducialFlag &d);

  /**
   * Reset the FiducialFlag object.
   */
  void Clear();

  /**
   * Return true/false if event is fiducial.
   * @return True if fiducial.
   */
  bool isFiducial() const { return is_fiducial_; }

  /**
   * Return fiducial flag bit mask.
   * @return Fiducial flag bit mask.
   */
  int getFiducialFlag() const { return fiducial_flag_; }

  /**
   * Return true/false if event has ecal hit.
   * @return True if ecal hit.
   */
  bool hasEcalHit() const { return has_ecal_hit_; }

  /**
   * Return true/false if event has hcal hit.
   * @return True if hcal hit.
   */
  bool hasHcalHit() const { return has_hcal_hit_; }

  /**
   * Return true/false if event has min. number of tracker hits.
   * @return True if >= min tracker hits.
   */
  bool hasMinTrackerHits() const { return has_min_tracker_hits_; }

  /**
   * Return true/false if event has min. recoil energy at production.
   * @return True if >= min energy.
   */
  bool hasMinEnergy() const { return has_min_energy_; }

  /**
   * Return algorithm variable i (see algorithm code for details).
   * @param element The index of the variable.
   * @return Algorithm variable at the index.
   */
  double getAlgoVar(int element) const { return variables_[element]; }

  /**
   * Return algorithm variable 0 (see algorithm code for details).
   * @note Provided for interactive ROOT use.
   * @return Algorithm variable 0.
   */
  double getAlgoVar0() const {
    return (variables_.GetSize() < 1) ? (0) : (variables_[0]);
  }

  /**
   * Return algorithm variable 1 (see algorithm code for details).
   * @note Provided for interactive ROOT use.
   * @return Algorithm variable 1.
   */
  double getAlgoVar1() const {
    return (variables_.GetSize() < 2) ? (0) : (variables_[1]);
  }

  /**
   * Return algorithm variable 2 (see algorithm code for details).
   * @note Provided for interactive ROOT use.
   * @return Algorithm variable 2.
   */
  double getAlgoVar2() const {
    return (variables_.GetSize() < 3) ? (0) : (variables_[2]);
  }

  /**
   * Return algorithm variable 3 (see algorithm code for details).
   * @note Provided for interactive ROOT use.
   * @return Algorithm variable 3.
   */
  double getAlgoVar3() const {
    return (variables_.GetSize() < 4) ? (0) : (variables_[3]);
  }

  /**
   * Return algorithm variable 4 (see algorithm code for details).
   * @note Provided for interactive ROOT use.
   * @return Algorithm variable 4.
   */
  double getAlgoVar4() const {
    return (variables_.GetSize() < 5) ? (0) : (variables_[4]);
  }

  /**
   * Set fiduciality bit mask.
   * @param fiducial_flag Bit-mask holding fiduciality conditions.
   * @param nvar The number of algorithm variables.
   */
  void setFiducialFlag(int fiducial_flag, int nvar);

  /**
   * Set fiduciality flag.
   * @param is_fiducial True/false if event is fiducial.
   */
  void setIsFiducial(bool is_fiducial) { is_fiducial_ = is_fiducial; }

  /**
   * Set ecal hit flag.
   * @param has_ecal_hit True/false if event has ecal hit.
   */
  void setHasEcalHit(bool has_ecal_hit) { has_ecal_hit_ = has_ecal_hit; }

  /**
   * Set hcal hit flag.
   * @param has_hcal_hit True/false if event has hcal hit.
   */
  void setHasHcalHit(bool has_hcal_hit) { has_hcal_hit_ = has_hcal_hit; }

  /**
   * Set tracker hit flag.
   * @param has_min_tracker_hits True/false if event has >= min. tracker hits.
   */
  void setHasMinTrackerHits(bool has_min_tracker_hits) {
    has_min_tracker_hits_ = has_min_tracker_hits;
  }

  /**
   * Set recoil min. energy flag.
   * @param has_min_energy True/false if event has >= min. recoil energy.
   */
  void setHasMinEnergy(bool has_min_energy) {
    has_min_energy_ = has_min_energy;
  }

  /**
   * Set an algorithm variable.
   * @param element The index of the variable.
   * @param value The variable's new value.
   */
  void setAlgoVar(int element, double value);

 private:
  /* Bit-mask that represents fiduciality conditions. */
  int fiducial_flag_{0};

  /* True if event is fiducial. */
  bool is_fiducial_{false};

  /* True if recoil electron has ecal hit. */
  bool has_ecal_hit_{false};

  /* True if recoil electron has hcal hit. */
  bool has_hcal_hit_{false};

  /* True if recoil electron has >= min tracker hits. */
  bool has_min_tracker_hits_{false};

  /* True if recoil electron has >= min energy at production. */
  bool has_min_energy_{false};

  /* Variable results from the fiduciality decision. */
  TArrayD variables_;

  ClassDef(FiducialFlag, 2);
};
}  // namespace ldmx

#endif

/**
 * @file TriggerResult.h
 * @brief Class that represents the trigger decision (pass/fail) for
 * reconstruction
 * @author Jeremy Mans, University of Minnesota
 */

#ifndef RECON_EVENT_TRIGGERRESULT_H_
#define RECON_EVENT_TRIGGERRESULT_H_

// ROOT
#include "TArrayD.h"
#include "TObject.h"  //ClassDef
#include "TString.h"

// STL
#include <iostream>

namespace ldmx {

/**
 * @class TriggerResult
 * @brief Represents the trigger decision (pass/fail) for reconstruction
 */
class TriggerResult {
 public:
  /**
   * Class constructor.
   */
  TriggerResult();

  /**
   * Class destructor.
   */
  virtual ~TriggerResult();

  /**
   * Print a description of this object.
   */
  void Print() const;

  /**
   * Reset the TriggerResult object.
   */
  void Clear();

  /**
   * Return the name of the trigger.
   * @return The name of the trigger.
   */
  const TString& getName() const { return name_; }

  /**
   * Return pass/fail status of the trigger.
   * @return True if trigger passed.
   */
  bool passed() const { return pass_; }

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
   * Set name and pass of trigger.
   * @param name The name of the trigger.
   * @param pass The pass/fail status of the trigger.
   * @param nvar The number of algorithm variables.
   */
  void set(const TString& name, bool pass, int nvar);

  /**
   * Set an algorithm variable.
   * @param element The index of the variable.
   * @param value The variable's new value.
   */
  void setAlgoVar(int element, double value);

 private:
  /** Name of the trigger algorithm. */
  TString name_{};

  /* Represents the pass/fail trigger decision. */
  bool pass_{false};

  /* Algorithm variable results from the trigger decision. */
  TArrayD variables_;

  ClassDef(TriggerResult, 1);
};
}  // namespace ldmx

#endif

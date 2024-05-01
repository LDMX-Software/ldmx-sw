/**
 * @file HcalVetoResult.h
 * @brief Class used to encapsulate the results obtained from
 *        HcalVetoProcessor.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef HCAL_EVENT_HCALVETORESULT_H_
#define HCAL_EVENT_HCALVETORESULT_H_

//----------//
//   ROOT   //
//----------//
#include "TObject.h"  //For ClassDef

//----------//
//   LDMX   //
//----------//
#include "Hcal/Event/HcalHit.h"

namespace ldmx {

class HcalVetoResult {
 public:
  /** Constructor */
  HcalVetoResult() = default;

  /** Destructor */
  virtual ~HcalVetoResult() = default;

  /** Reset the object. */
  void Clear();

  /** Print out the object */
  void Print() const;

  /** Checks if the event passes the Hcal veto. */
  bool passesVeto() const { return passesVeto_; };

  /** @return The maximum PE HcalHit. */
  ldmx::HcalHit getMaxPEHit() const { return maxPEHit_; }

  /**
   * Sets whether the Hcal veto was passed or not.
   *
   * @param passesVeto Veto result.
   */
  void setVetoResult(const bool& passesVeto = true) {
    passesVeto_ = passesVeto;
  }

  /**
   * Set the maximum PE hit.
   *
   * @param maxPEHit The maximum PE HcalHit
   */
  void setMaxPEHit(const ldmx::HcalHit maxPEHit) { maxPEHit_ = maxPEHit; }

 private:
  /** Reference to max PE hit. */
  ldmx::HcalHit maxPEHit_;

  /** Flag indicating whether the event passes the Hcal veto. */
  bool passesVeto_{false};

  ClassDef(HcalVetoResult, 2);

};  // HcalVetoResult
}  // namespace ldmx

#endif  // HCAL_EVENT_HCALVETORESULT_H_

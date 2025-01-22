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
  bool passesVeto() const { return passes_veto_; };

  /** @return The maximum PE HcalHit. */
  ldmx::HcalHit getMaxPEHit() const { return max_PE_hit_; }

  /** @return The total number of PE. */
  float getTotalPE() const { return total_PE_; }

  /** @return The number of valid hits. */
  float getNumValidHits() const { return num_valid_hits_; }
  
  /**
   * Sets whether the Hcal veto was passed or not.
   *
   * @param passes_veto Veto result.
   */
  void setVetoResult(const bool& passes_veto = true) {
    passes_veto_ = passes_veto;
  }

  /**
   * Set the maximum PE hit.
   *
   * @param max_PE_hit The maximum PE HcalHit
   */
  void setMaxPEHit(const ldmx::HcalHit max_PE_hit) { max_PE_hit_ = max_PE_hit; }

  /**
   * Set the total number of PE.
   *
   * @param total_PE The maximum PE HcalHit
   */
  void setTotalPE(const float total_PE) { total_PE_ = total_PE; }


  /**
   * Set the number of valid hits.
   *
   * @param num_valid_hits The number of valid hits
   */
  void setNumValidHits(const float num_valid_hits) { num_valid_hits_ = num_valid_hits; }
  

 private:
  /** Reference to max PE hit. */
  ldmx::HcalHit max_PE_hit_;

  // Total number of PE
  float total_PE_{0.0};

  // Number of hits above threshold
  int num_valid_hits_{0};

  /** Flag indicating whether the event passes the Hcal veto. */
  bool passes_veto_{false};

  ClassDef(HcalVetoResult, 3);

};  // HcalVetoResult
}  // namespace ldmx

#endif  // HCAL_EVENT_HCALVETORESULT_H_

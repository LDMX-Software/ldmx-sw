/**
 * @file HcalHit.h
 * @brief Class that stores Stores reconstructed hit information from the HCAL
 * @author Jeremy Mans, University of Minnesota
 */

#ifndef HCAL_EVENT_HCALHIT_H_
#define HCAL_EVENT_HCALHIT_H_

// LDMX
#include "Recon/Event/CalorimeterHit.h"

namespace hcal {
namespace event {

/**
 * @class HcalHit
 * @brief Stores reconstructed hit information from the HCAL
 *
 * @note This class represents the reconstructed hit information
 * from the HCAL, providing particular information for the HCAL,
 * above and beyond what is available in the CalorimeterHit.
 */
class HcalHit : public recon::event::CalorimeterHit {
 public:
  /**
   * Class constructor.
   */
  HcalHit() {}

  /**
   * Class destructor.
   */
  virtual ~HcalHit() {}

  /**
   * Clear the data in the object.
   */
  void Clear();

  /**
   * Print out the object.
   */
  void Print() const;

  /**
   * Get the number of photoelectrons estimated for this hit.
   * @return Number of photoelectrons, including noise which affects the
   * estimate.
   */
  float getPE() const { return pe_; }

  /**
   * Get the minimum number of photoelectrons estimated for this hit if two
   * sided readout.
   * @return Minimum number of photoelectrons, including noise which affects the
   * estimate.
   */
  float getMinPE() const { return minpe_; }

  /**
   * Set the number of photoelectrons estimated for this hit.
   * @param pe Number of photoelectrons, including noise which affects the
   * estimate.
   */
  void setPE(float pe) { pe_ = pe; }

  /**
   * Set the minimum number of photoelectrons estimated for this hit.
   * @param pe Minimum number of photoelectrons, including noise which affects
   * the estimate.
   */
  void setMinPE(float minpe) { minpe_ = minpe; }

 private:
  /** The number of PE estimated for this hit. */
  float pe_{0};

  /** The minimum number of PE estimated for this hit, different from pe_ when
   * you have two ended readout */
  float minpe_{-99};

  /**
   * The ROOT class definition.
   */
  ClassDef(HcalHit, 2);
};
}
}  // namespace hcal

#endif /* HCAL_EVENT_HCALHIT_H_ */

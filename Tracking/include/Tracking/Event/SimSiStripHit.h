
#ifndef TRACKING_EVENT_SIMSISTRIPHIT_H_
#define TRACKING_EVENT_SIMSISTRIPHIT_H_

//----------------------//
//   C++ Standard Lib   //
//----------------------//
#include <iostream>
#include <vector>

//----------//
//   LDMX   //
//----------//
#include "Tracking/Event/SiStripHit.h"

namespace ldmx {

/**
 * Truth (Monte Carlo) representation of a silicon strip detector hit.
 *
 * In addition to the ADC samples and time stamp held by the SiStripHit base
 * class, this class carries the truth-level deposited charge and energy. The
 * reco-level (real data) counterpart is RawSiStripHit.
 */
class SimSiStripHit : public SiStripHit {
 public:
  /// Default constructor
  SimSiStripHit() = default;

  /**
   * Constructor.
   *
   * The truth charge and energy are populated through the dedicated setters.
   *
   * @param[in] samples The ADC samples composing this hit.  For now, the size
   *    of a sample is assumed to be 16 bits.
   * @param[in] time The timestamp of this hit as set by the data acquisition
   *    system.
   */
  SimSiStripHit(std::vector<short> samples, long time);

  /**
   * Destructor.
   *
   * Currently, the destructor does nothing.
   */
  virtual ~SimSiStripHit() = default;

  /**
   * Clear the samples, time stamp and truth fields.
   *
   * This method is needed by ROOT when building the dictionary.
   */
  void clear() override;

  /// Get the truth (MC) deposited charge.
  float getCharge() const { return charge_; }
  /// Get the truth (MC) deposited energy.
  float getEdep() const { return edep_; }

  /// Set the truth (MC) deposited charge.
  void setCharge(float v) { charge_ = v; }
  /// Set the truth (MC) deposited energy.
  void setEdep(float v) { edep_ = v; }

  /**
   * Overload the stream insertion operator to output a string representation
   * of this SimSiStripHit.
   *
   * @param[in] output The output stream where the string representation will
   *    be inserted.
   * @param[in] hit The SimSiStripHit to output.
   *
   * @return An ostream object with the string representation of
   *    SimSiStripHit inserted.
   */
  friend std::ostream &operator<<(std::ostream &output,
                                  const SimSiStripHit &hit);

 protected:
  /// Truth (MC) deposited charge.
  float charge_{0};

  /// Truth (MC) deposited energy.
  float edep_{0};

  /// Class declaration needed by the ROOT dictionary.
  ClassDefOverride(SimSiStripHit, 1);

};  // SimSiStripHit
}  // namespace ldmx

#endif  // TRACKING_EVENT_SIMSISTRIPHIT_H_

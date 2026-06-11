
#ifndef TRACKING_EVENT_SISTRIPHIT_H_
#define TRACKING_EVENT_SISTRIPHIT_H_

//----------------------//
//   C++ Standard Lib   //
//----------------------//
#include <iostream>
#include <vector>

//----------//
//   ROOT   //
//----------//
#include "TObject.h"

namespace ldmx {

/**
 * Abstract base class for a silicon strip detector hit.
 *
 * This class holds the information that is common to both the raw (reco/data)
 * and simulated (truth) representations of a silicon strip hit: the ADC samples
 * and the hit time stamp. The reco-level electronics and quality information
 * lives in RawSiStripHit; the truth-level (MC) information lives in
 * SimSiStripHit.
 */
class SiStripHit {
 public:
  /// Default constructor
  SiStripHit() = default;

  /**
   * Destructor.
   *
   * Currently, the destructor does nothing.
   */
  virtual ~SiStripHit() = default;

  /**
   * Clear the data in the object.
   *
   * Pure virtual so that SiStripHit cannot be instantiated on its own; each
   * concrete hit type is responsible for clearing its own fields (and the
   * shared base fields via clearBase()).
   */
  virtual void clear() = 0;

  /**
   * Get the digitized (ADC) samples composing this hit.
   *
   * This can be a single value or multiple values depending on the readout
   * being used.
   *
   * @return A std::vector of 16 bit samples.
   */
  std::vector<short> getSamples() const { return samples_; }

  /**
   * Get the time stamp of this hit.
   *
   * This is the time stamp as set by the data acquisition system. This will
   * typically be in units of ns.
   *
   * @return The timestamp of this hit in ns.
   */
  long getTime() const { return time_; }

  /**
   * When the less than operator is used for comparison, return true if this
   * hit's time is less than the hit we are comparing against.
   *
   * @param[in] rhs The SiStripHit on the right side of the comparison.
   *
   * @return True if the timestamp of this hit is less than the hit being
   *    compared against.
   */
  bool operator<(const SiStripHit &rhs) const {
    return getTime() < rhs.getTime();
  }

 protected:
  /**
   * Constructor used by derived classes to initialize the shared fields.
   *
   * @param[in] samples The ADC samples composing this hit. For now, the size
   *    of a sample is assumed to be 16 bits.
   * @param[in] time The timestamp of this hit as set by the data acquisition
   *    system.
   */
  SiStripHit(std::vector<short> samples, long time)
      : samples_(samples), time_(time) {}

  /// Clear the fields owned by the base class.
  void clearBase() {
    samples_.clear();
    time_ = 0;
  }

  /// 16 bit ADC samples associated with this hit.
  std::vector<short> samples_;

  /// The hit time stamp in units of ns.
  long time_{0};

  /// Class declaration needed by the ROOT dictionary.
  ClassDef(SiStripHit, 1);

};  // SiStripHit
}  // namespace ldmx

#endif  // TRACKING_EVENT_SISTRIPHIT_H_

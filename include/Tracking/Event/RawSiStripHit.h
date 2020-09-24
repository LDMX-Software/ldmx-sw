
#ifndef TRACKING_EVENT_RAWSISTRIPHIT_H_
#define TRACKING_EVENT_RAWSISTRIPHIT_H_

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
 * Implementation of a raw digitized hit from a silicon strip detector.
 *
 * This class is meant to encapsulate the raw data coming from a silicon strip
 * detector prior to any additional processing. Typically, the raw data will
 * contain a header with ID information and timestamp, a tail with error info
 * and multiple 32 bit data samples.
 */
class RawSiStripHit {
public:
  /// Default constructor
  RawSiStripHit() = default;

  /**
   * Constructor.
   *
   * @param[in] samples The ADC samples composing this hit.  For now, the size
   *    of a sample is assumed to be 16 bits. 
   * @param[in] time The timestamp of this hit as set by the data acquisition
   *    system.
   */
  RawSiStripHit(std::vector<short> samples, long time); 

  /**
   * Destructor.
   *
   * Currently, the destructor does nothing.
   */
  virtual ~RawSiStripHit(){};

  /**
   * Clear the vector of samples and set the timestamp to 0. 
   *
   * This class is needed by ROOT when building the dictionary. 
   */
   void Clear(); 

   /**
    * Print the string representation of this object. 
    *
    * This class is needed by ROOT when building the dictionary. 
    */
   void Print() const { std::cout << this; }


  /**
   * Get the digitized (ADC) samples composing this hit.
   *
   * This can be a single value or multiple values depending on the readout
   * being used.
   *
   * @param[in] samples_ The ADC values composing this hit. For now, the size
   *    of a sample is assumed to be 16 bits.
   *
   * @return[out] A std::vector of 16 bit samples.
   */
  std::vector<short> getSamples() const { return samples_; }

  /**
   * Get the time stamp of this hit.
   *
   * This is the time stamp as set by the data aquisition system. This will
   * typically be in units of ns.
   *
   * @param[in] time_ The timestamp as set by the data acquisition system.
   *
   * @return[out] The timestamp of this hit in ns.
   */
  long getTime() const { return time_; }

  /**
   * When the less than operator is used for comparison, return true if this
   * hit's time is less than the hit we are comparing against.
   *
   * @param[in] rhs The RawStripHit on the right side of the comparison.
   *
   * @return[out] True if the timestamp of this hit is less than the hit being
   *    compared against.
   */
  bool operator<(const RawSiStripHit &rhs) const {
    return getTime() < rhs.getTime();
  }

  /**
   * Overload the stream insertion operator to output a string representation
   * of this RawStripHit.
   *
   * @param[in] output The output stream where the string representation will
   *    be inserted.
   * @param[in] hit The RawSiStripHit to output.
   *
   * @return[out] An ostream object with the string representation of
   *    RawSiStripHit inserted.
   */
  friend std::ostream &operator<<(std::ostream &output,
                                  const RawSiStripHit &hit);

protected:
  /// 16 bit ADC samples associated with this hit.
  std::vector<short> samples_;

  /// The hit time stamp in units of ns.
  long time_{0};

  /// Class declaration needed by the ROOT dictionary.
  ClassDef(RawSiStripHit, 1);

}; // RawSiStripHit
} // namespace ldmx

#endif // TRACKING_EVENT_RAWSISTRIPHIT_H_

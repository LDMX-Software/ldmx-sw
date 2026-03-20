
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
   * Constructor with full electronics ID and quality fields from raw Rogue
   * frame data.
   */
  RawSiStripHit(std::vector<short> samples, long time,
                uint8_t channel, uint8_t apv_id, uint8_t hybrid_id,
                uint8_t feb_id, uint16_t apv_trigger,
                uint8_t read_error, uint8_t head, uint8_t tail, uint8_t filter);

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
  void clear();

  /**
   * Print the string representation of this object.
   *
   * This class is needed by ROOT when building the dictionary.
   */
  friend std::ostream &operator<<(std::ostream &o, const RawSiStripHit &d);

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

  uint8_t  getChannel()    const { return channel_; }
  uint8_t  getApvId()      const { return apv_id_; }
  uint8_t  getHybridId()   const { return hybrid_id_; }
  uint8_t  getFebId()      const { return feb_id_; }
  uint16_t getApvTrigger() const { return apv_trigger_; }
  uint8_t  getReadError()  const { return read_error_; }
  uint8_t  getHead()       const { return head_; }
  uint8_t  getTail()       const { return tail_; }
  uint8_t  getFilter()     const { return filter_; }

  void setChannel(uint8_t v)      { channel_ = v; }
  void setApvId(uint8_t v)        { apv_id_ = v; }
  void setHybridId(uint8_t v)     { hybrid_id_ = v; }
  void setFebId(uint8_t v)        { feb_id_ = v; }
  void setApvTrigger(uint16_t v)  { apv_trigger_ = v; }
  void setReadError(uint8_t v)    { read_error_ = v; }
  void setHead(uint8_t v)         { head_ = v; }
  void setTail(uint8_t v)         { tail_ = v; }
  void setFilter(uint8_t v)       { filter_ = v; }

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

  uint8_t  channel_{0};
  uint8_t  apv_id_{0};
  uint8_t  hybrid_id_{0};
  uint8_t  feb_id_{0};
  uint16_t apv_trigger_{0};
  uint8_t  read_error_{0};
  uint8_t  head_{0};
  uint8_t  tail_{0};
  uint8_t  filter_{0};

  /// Class declaration needed by the ROOT dictionary.
  ClassDef(RawSiStripHit, 3);

};  // RawSiStripHit
}  // namespace ldmx

#endif  // TRACKING_EVENT_RAWSISTRIPHIT_H_


#ifndef TRACKING_EVENT_RAWSISTRIPHIT_H_
#define TRACKING_EVENT_RAWSISTRIPHIT_H_

//----------------------//
//   C++ Standard Lib   //
//----------------------//
#include <cstdint>
#include <iostream>
#include <vector>

//----------//
//   LDMX   //
//----------//
#include "Tracking/Event/SiStripHit.h"

namespace ldmx {

/**
 * Implementation of a raw digitized hit from a silicon strip detector.
 *
 * This class encapsulates the reco-level (real data) information for a silicon
 * strip hit: in addition to the ADC samples and time stamp held by the
 * SiStripHit base class, it carries the electronics identifiers (channel, APV,
 * hybrid, FEB) and quality/error flags read out of the raw Rogue frame. The
 * truth-level counterpart is SimSiStripHit.
 */
class RawSiStripHit : public SiStripHit {
 public:
  /// Default constructor
  RawSiStripHit() = default;

  /**
   * Constructor.
   *
   * The remaining electronics and quality fields are populated through the
   * dedicated setters.
   *
   * @param[in] channel The readout channel of this hit.
   * @param[in] samples The ADC samples composing this hit.  For now, the size
   *    of a sample is assumed to be 16 bits.
   * @param[in] time The timestamp of this hit as set by the data acquisition
   *    system.
   */
  RawSiStripHit(uint8_t channel, std::vector<short> samples, long time);

  /**
   * Destructor.
   *
   * Currently, the destructor does nothing.
   */
  virtual ~RawSiStripHit() = default;

  /**
   * Clear the samples, time stamp and electronics/quality fields.
   *
   * This method is needed by ROOT when building the dictionary.
   */
  void clear() override;

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
   * Overload the stream insertion operator to output a string representation
   * of this RawSiStripHit.
   *
   * @param[in] output The output stream where the string representation will
   *    be inserted.
   * @param[in] hit The RawSiStripHit to output.
   *
   * @return An ostream object with the string representation of
   *    RawSiStripHit inserted.
   */
  friend std::ostream &operator<<(std::ostream &output,
                                  const RawSiStripHit &hit);

 protected:
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
  ClassDefOverride(RawSiStripHit, 4);

};  // RawSiStripHit
}  // namespace ldmx

#endif  // TRACKING_EVENT_RAWSISTRIPHIT_H_

#ifndef PACKING_DAQSAMPLEHEADER_H
#define PACKING_DAQSAMPLEHEADER_H

#include <cstdint>
#include <ostream>

namespace packing {

/**
 * The capture header that the DAQ firmware inserts before each
 * ECON-D event packet (one per sample) and the trailer word
 * that it appends after the last sample of an event.
 *
 * Header:
 *
 *     4b vers | 10b ECON ID | 5b i_l1a | S | 12b length
 *
 * - vers is the format version (1)
 * - ECON ID is what it was configured in the software to be
 * - i_l1a is the index of the sample relative to this event
 * - S signals if this is the sample of interest (1) or not (0)
 * - length is the length of the ECON-D event packet in 32b words
 *   NOT including this header (= ECON-D payload length + 2)
 *
 * Trailer:
 *
 *     4b vers | 10b 0x3ff | 5b 0x1f | 0 | last | 6b 0 | 5b n_samples
 *
 * @note Ported from pflib::packing::DAQSampleHeader.
 */
struct DAQSampleHeader {
  /// format version
  uint32_t version_{0};
  /// configured ID of the ECON
  uint32_t econd_id_{0};
  /// index of this sample within the event
  uint32_t i_l1a_{0};
  /// is this the sample of interest?
  bool is_soi_{false};
  /// length of the ECON-D packet following this header in 32b words
  uint32_t econd_len_{0};

  /// expected format version
  static const uint32_t VERSION{1};

  /// decode a DAQSampleHeader from the input word
  void from(uint32_t word);

  /// construct a header word from our data
  uint32_t to() const;

  /**
   * check if the decoded word looks like a capture header
   *
   * The version must be the expected one and the three bits
   * between the S flag and the 9-bit length must be zero.
   * The trailer does not pass this check.
   */
  static bool isValid(uint32_t word);

  /**
   * A special form of this DAQ header is used to signal
   * the end of a multi-sample sequence.
   *
   * Both i_l1a and econd_id are set to their maximum values.
   */
  bool isEndingTrailer() const;

  /// number of samples written in the ending trailer
  static uint32_t trailerNumSamples(uint32_t word);

  /// construct the special trailer form of this header
  static uint32_t endingTrailer(uint32_t n_samples = 0);

  /// output stream operator to make logging easier
  friend std::ostream& operator<<(std::ostream& o, const DAQSampleHeader& h);
};

}  // namespace packing

#endif  // PACKING_DAQSAMPLEHEADER_H

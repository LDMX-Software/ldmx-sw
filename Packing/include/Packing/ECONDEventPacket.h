#ifndef PACKING_ECONDEVENTPACKET_H
#define PACKING_ECONDEVENTPACKET_H

#include <array>
#include <cstdint>
#include <span>
#include <vector>

#include "Framework/Logger.h"

namespace packing {

/**
 * Data from a single eRx ("link") sub-packet of an ECON-D event packet
 *
 * The channel words are stored in the HGCROC 32-bit sample format
 * (the same format that ldmx::HgcrocDigiCollection::Sample decodes).
 * In pass through mode they are copied from the link untouched,
 * otherwise they are rebuilt from the zero-suppressed measurements
 * with suppressed measurements set to zero.
 */
struct ECONDLinkSubpacket {
  /// 3-bit Stat field (HT, EBO, CRC match), all ones is good
  uint32_t stat_{0};
  /// 3-bit Hamming code of the sub-packet header
  uint32_t hamming_{0};
  /// F flag: sub-packet is a single header word without channel data
  bool is_empty_{false};
  /// E flag of an empty sub-packet: suppressed due to unmasked Stat errors
  bool error_caused_empty_{false};
  /// ADC readout of common mode 0
  int adc_cm0_{0};
  /// ADC readout of common mode 1
  int adc_cm1_{0};
  /// 37-bit channel map (bit 18 is the calibration channel)
  uint64_t channel_map_{0};
  /// sample words of the 36 normal channels
  std::array<uint32_t, 36> channels_{};
  /// sample word of the calibration channel
  uint32_t calib_{0};

  /// any of the three Stat bits are bad (not 1)
  bool badStat() const { return stat_ != 0b111; }
};

/**
 * Single ECON-D event packet
 *
 * The ECON-D is the CONcentrator mezzanine for the Data path
 * and it has many different configuration modes.
 * The two most important to us for decoding are
 * 1. "Pass Through" mode - all 37 sample words are copied from the link,
 *    the link headers from the ROC are overwritten by ECOND link headers
 * 2. "Regular" mode - the ECOND event format is written which can include
 *    zero-suppression of multiple varieties including full eliminating
 *    channels that are empty
 *
 * The format is described in Section 18 of the ECON-D specification
 * (ECON-D_specification_working_doc_v1.4_5jan2023.pdf).
 *
 * @note Ported from pflib::packing::ECONDEventPacket with bounds checks
 * added so that corrupted packets are flagged instead of read past.
 */
class ECONDEventPacket {
 public:
  /// index of corruption flags
  enum Corruption : std::size_t {
    /// 9b header marker is not the ECOND spec or common CMS config
    BAD_HEADER_MARKER = 0,
    /// stored payload length does not equal packet length
    BAD_LENGTH = 1,
    /// 8b header CRC mismatch
    BAD_HEADER_CRC = 2,
    /// 32b sub-packet CRC mismatch
    BAD_CRC32 = 3,
    /// link sub-packets do not fit or do not end at the CRC word
    BAD_PAYLOAD = 4,
    /// T flag set, link sub-packets not transmitted
    TRUNCATED = 5,
    /// number of corruption flags
    N_CORRUPTION = 6
  };

  /// corruption flags indexed by Corruption
  std::array<bool, N_CORRUPTION> corruption_{};

  /// 9b header marker
  uint32_t header_marker_{0};
  /// payload length in 32b words (not including the two header words)
  uint32_t payload_length_{0};
  /// P: pass through mode
  bool passthrough_{false};
  /// E: HGCROC event header/trailer expected
  bool expected_{false};
  /// H/T: status of HGCROC header/trailer reconstruction
  uint32_t ht_{0};
  /// E/B/O: status of event/BX/orbit reconstruction
  uint32_t ebo_{0};
  /// M: transmitted E/B/O matches the L1A FIFO
  bool match_{false};
  /// T: packet truncated for buffer overflow
  bool truncated_{false};
  /// 6b Hamming code of the header
  uint32_t hamming_{0};
  /// bunch crossing
  uint32_t bx_{0};
  /// 6b L1A (event) number
  uint32_t l1a_{0};
  /// 3b orbit number
  uint32_t orbit_{0};
  /// S: error in a link sub-packet
  bool subpacket_error_{false};
  /// RR: reset request
  uint32_t reset_request_{0};
  /// transmitted 8b header CRC
  uint32_t crc8_{0};

  /**
   * storage of unpacked link data
   *
   * This is very space in-efficient since we store zero words for
   * any channel or measurement that was zero suppressed.
   */
  std::vector<ECONDLinkSubpacket> links_;

  /// construct an event packet with the number of eRx "links"
  explicit ECONDEventPacket(std::size_t n_links);

  /// payload length of zero, no link sub-packets
  bool isEmpty() const { return payload_length_ == 0; }

  /// any of the corruption flags are set
  bool isCorrupt() const;

  /**
   * parse into the packet from the passed data span
   *
   * The span should start at the first ECON-D header word and contain
   * the full packet (as given by the DAQ capture header length).
   * This function never reads outside of the span. On corruption,
   * the corresponding flag is set and parsing stops early.
   *
   * @return true if the packet is not corrupt
   */
  bool from(std::span<const uint32_t> data);

  /// spec default and common CMS header markers (8b marker + 0)
  static const uint32_t HEADER_MARKER_SPEC{0xaa << 1};
  static const uint32_t HEADER_MARKER_CMS{0xf3 << 1};

 private:
  /**
   * Unpack a eRx ("link") sub-packet
   *
   * @param[in] data words from the start of the sub-packet to the CRC word
   * (not included)
   * @param[out] link sub-packet to update with unpacked data
   * @return length of data in 32b words that was unpacked, 0 if the
   * sub-packet does not fit into data
   */
  std::size_t unpackLinkSubpacket(std::span<const uint32_t> data,
                                  ECONDLinkSubpacket& link) const;

  /// handle to logging source
  enableLogging("ECONDEventPacket");
};

}  // namespace packing

#endif  // PACKING_ECONDEVENTPACKET_H

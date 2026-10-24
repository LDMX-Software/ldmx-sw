#ifndef PACKING_MULTISAMPLEECONDEVENTPACKET_H
#define PACKING_MULTISAMPLEECONDEVENTPACKET_H

#include "Packing/DAQSampleHeader.h"
#include "Packing/ECONDEventPacket.h"

namespace packing {

/**
 * Unpack an event that has potentially more than one sample
 * collected from a **single** ECOND.
 *
 * The frame is what SingleSubsystemUnpacker puts on the event bus
 * (the data after the LDMX RoR header) as 32-bit words:
 * for each sample a DAQSampleHeader followed by the ECON-D packet,
 * then the DAQSampleHeader::endingTrailer and zero padding.
 *
 * Corrupted ECON-D packets are flagged in their ECONDEventPacket and
 * we move on to the next capture header using the length.
 * If the word at that position is not a capture header, we search
 * forward for the capture header of the next sample (or the trailer)
 * and count the skipped words.
 *
 * @see ECONDEventPacket for how a single sample from a single ECOND
 * is unpacked.
 *
 * @note Ported from pflib::packing::MultiSampleECONDEventPacket.
 */
class MultiSampleECONDEventPacket {
 public:
  /// index of frame-level corruption flags
  enum Corruption : std::size_t {
    /// a capture header was missing, words were skipped to find the next
    RESYNC = 0,
    /// no further capture header or trailer found, rest of frame dropped
    LOST_SYNC = 1,
    /// the frame ended without the ending trailer
    NO_TRAILER = 2,
    /// number of samples in the trailer does not match the number found
    TRAILER_COUNT_MISMATCH = 3,
    /// ECON ID changed within the frame
    ECON_ID_MISMATCH = 4,
    /// sample index in capture header does not match its position
    SAMPLE_INDEX_MISMATCH = 5,
    /// capture header length goes past the end of the frame
    TRUNCATED_FRAME = 6,
    /// non-zero words after the ending trailer
    EXTRA_WORDS = 7,
    /// no sample flagged as the sample of interest
    NO_SOI = 8,
    /// number of corruption flags
    N_CORRUPTION = 9
  };

  /// corruption flags indexed by Corruption
  std::array<bool, N_CORRUPTION> corruption_{};
  /// index of the sample of interest (SOI)
  std::size_t i_soi_{0};
  /// ID for this econ from the first capture header (-1 if none)
  int econd_id_{-1};
  /// number of samples written in the trailer (-1 if no trailer)
  int trailer_n_samples_{-1};
  /// number of words skipped while searching for capture headers
  std::size_t n_skipped_words_{0};
  /// capture headers stored in order of transmission
  std::vector<DAQSampleHeader> headers_;
  /// samples from ECOND stored in order of transmission
  std::vector<ECONDEventPacket> samples_;

  /// constructor defining how many links are connected to this ECOND
  explicit MultiSampleECONDEventPacket(int n_links);

  /// get the sample of interest
  const ECONDEventPacket& soi() const;

  /// any of the frame-level corruption flags are set
  bool isCorrupt() const;

  /// unpack the given frame into this structure
  void from(std::span<const uint32_t> frame);

 private:
  /// find the next capture header for sample i_sample or the trailer
  std::size_t resync(std::span<const uint32_t> frame, std::size_t start,
                     std::size_t i_sample) const;

  /// number of links connected to the ECOND
  int n_links_;
  /// handle to logging source
  enableLogging("MultiSampleECONDEventPacket");
};

}  // namespace packing

#endif  // PACKING_MULTISAMPLEECONDEVENTPACKET_H

/**
 * @file ECONDEventInfo.h
 * @brief Classes holding the ECON-D packet data that does not fit into
 * an HgcrocDigiCollection
 */

#ifndef RECON_EVENT_ECONDEVENTINFO_H_
#define RECON_EVENT_ECONDEVENTINFO_H_

// ROOT
#include <Rtypes.h>  // For ClassDef

// STL
#include <cstdint>
#include <iostream>
#include <vector>

namespace ldmx {

/**
 * @class ECONDLinkInfo
 * @brief Data from one eRx ("link") sub-packet of an ECON-D packet
 * that is not a normal channel
 */
class ECONDLinkInfo {
 public:
  ECONDLinkInfo() = default;
  virtual ~ECONDLinkInfo() = default;

  /**
   * Set the link data
   *
   * @param[in] stat 3-bit Stat field (HT, EBO, CRC match)
   * @param[in] is_empty F flag, no channel data in this sub-packet
   * @param[in] adc_cm0 common mode 0 ADC
   * @param[in] adc_cm1 common mode 1 ADC
   * @param[in] calib calibration channel sample word
   */
  void set(int stat, bool is_empty, int adc_cm0, int adc_cm1, uint32_t calib) {
    stat_ = stat;
    is_empty_ = is_empty;
    adc_cm0_ = adc_cm0;
    adc_cm1_ = adc_cm1;
    calib_ = calib;
  }

  /// 3-bit Stat field, all ones is good
  int getStat() const { return stat_; }
  /// any of the Stat bits are bad
  bool hasBadStat() const { return stat_ != 0b111; }
  /// sub-packet had no channel data
  bool isEmpty() const { return is_empty_; }
  /// common mode 0 ADC
  int getADCCM0() const { return adc_cm0_; }
  /// common mode 1 ADC
  int getADCCM1() const { return adc_cm1_; }
  /// calibration channel sample word (HgcrocDigiCollection::Sample format)
  uint32_t getCalib() const { return calib_; }

  friend std::ostream& operator<<(std::ostream& o, const ECONDLinkInfo& l);

 private:
  /// 3-bit Stat field
  int stat_{0};
  /// F flag
  bool is_empty_{false};
  /// common mode 0 ADC
  int adc_cm0_{0};
  /// common mode 1 ADC
  int adc_cm1_{0};
  /// calibration channel sample word
  uint32_t calib_{0};

  ClassDef(ECONDLinkInfo, 1);
};

/**
 * @class ECONDSampleInfo
 * @brief Capture header and ECON-D header data for one sample (L1A)
 */
class ECONDSampleInfo {
 public:
  /**
   * Bits of the corruption mask
   *
   * Same order as packing::ECONDEventPacket::Corruption
   */
  enum Corruption {
    BAD_HEADER_MARKER = 0,
    BAD_LENGTH = 1,
    BAD_HEADER_CRC = 2,
    BAD_CRC32 = 3,
    BAD_PAYLOAD = 4,
    TRUNCATED = 5
  };

  ECONDSampleInfo() = default;
  virtual ~ECONDSampleInfo() = default;

  /**
   * Set the capture header data
   *
   * @param[in] econ_id ECON ID
   * @param[in] i_sample sample index within the event
   * @param[in] is_soi sample of interest flag
   * @param[in] length ECON-D packet length in 32b words
   */
  void setCaptureHeader(int econ_id, int i_sample, bool is_soi, int length) {
    econ_id_ = econ_id;
    i_sample_ = i_sample;
    is_soi_ = is_soi;
    capture_length_ = length;
  }

  /**
   * Set the ECON-D event header data
   *
   * The flags are packed as
   * P (bit 0), E (1), M (2), T (3), S (4).
   */
  void setHeader(int payload_length, int flags, int ht, int ebo, int bx,
                 int l1a, int orbit, int reset_request) {
    payload_length_ = payload_length;
    flags_ = flags;
    ht_ = ht;
    ebo_ = ebo;
    bx_ = bx;
    l1a_ = l1a;
    orbit_ = orbit;
    reset_request_ = reset_request;
  }

  /// set the corruption mask
  void setCorruption(int mask) { corruption_ = mask; }

  /// add the data from the next link
  void addLink(const ECONDLinkInfo& link) { links_.push_back(link); }

  /// ECON ID from the capture header
  int getEconID() const { return econ_id_; }
  /// sample index from the capture header
  int getSampleIndex() const { return i_sample_; }
  /// sample of interest flag from the capture header
  bool isSOI() const { return is_soi_; }
  /// ECON-D packet length from the capture header
  int getCaptureLength() const { return capture_length_; }
  /// payload length from the ECON-D header
  int getPayloadLength() const { return payload_length_; }
  /// P flag, pass through mode
  bool isPassthrough() const { return flags_ & 0b1; }
  /// E flag, HGCROC event header/trailer expected
  bool isExpected() const { return (flags_ >> 1) & 0b1; }
  /// M flag, E/B/O match with the L1A FIFO
  bool isMatch() const { return (flags_ >> 2) & 0b1; }
  /// T flag, truncated
  bool isTruncated() const { return (flags_ >> 3) & 0b1; }
  /// S flag, sub-packet error
  bool hasSubpacketError() const { return (flags_ >> 4) & 0b1; }
  /// 2-bit H/T status
  int getHT() const { return ht_; }
  /// 2-bit E/B/O status
  int getEBO() const { return ebo_; }
  /// bunch crossing
  int getBX() const { return bx_; }
  /// 6-bit L1A number
  int getL1A() const { return l1a_; }
  /// 3-bit orbit number
  int getOrbit() const { return orbit_; }
  /// 2-bit reset request
  int getResetRequest() const { return reset_request_; }
  /// corruption mask, bits from Corruption
  int getCorruption() const { return corruption_; }
  /// check a single corruption bit
  bool isCorrupt(Corruption c) const { return (corruption_ >> c) & 0b1; }
  /// any corruption bit is set
  bool isCorrupt() const { return corruption_ != 0; }
  /// link data in order of transmission
  const std::vector<ECONDLinkInfo>& getLinks() const { return links_; }

  friend std::ostream& operator<<(std::ostream& o, const ECONDSampleInfo& s);

 private:
  /// ECON ID
  int econ_id_{-1};
  /// sample index
  int i_sample_{-1};
  /// sample of interest flag
  bool is_soi_{false};
  /// ECON-D packet length from capture header
  int capture_length_{0};
  /// payload length from ECON-D header
  int payload_length_{0};
  /// packed P, E, M, T, S flags
  int flags_{0};
  /// H/T status
  int ht_{0};
  /// E/B/O status
  int ebo_{0};
  /// bunch crossing
  int bx_{0};
  /// L1A number
  int l1a_{0};
  /// orbit number
  int orbit_{0};
  /// reset request
  int reset_request_{0};
  /// corruption mask
  int corruption_{0};
  /// link data
  std::vector<ECONDLinkInfo> links_;

  ClassDef(ECONDSampleInfo, 1);
};

/**
 * @class ECONDEventInfo
 * @brief Supporting data for one event decoded from a single ECON-D
 *
 * Holds the frame-level checks and the per-sample data that
 * HgcrocDigiCollection does not store.
 */
class ECONDEventInfo {
 public:
  /**
   * Bits of the frame corruption mask
   *
   * Same order as packing::MultiSampleECONDEventPacket::Corruption
   */
  enum Corruption {
    RESYNC = 0,
    LOST_SYNC = 1,
    NO_TRAILER = 2,
    TRAILER_COUNT_MISMATCH = 3,
    ECON_ID_MISMATCH = 4,
    SAMPLE_INDEX_MISMATCH = 5,
    TRUNCATED_FRAME = 6,
    EXTRA_WORDS = 7,
    NO_SOI = 8,
    BAD_FRAME_SIZE = 9
  };

  ECONDEventInfo() = default;
  virtual ~ECONDEventInfo() = default;

  /// reset to the default state
  void clear();

  /// set the frame-level data
  void setFrame(int corruption, int trailer_n_samples, int i_soi,
                int n_skipped_words) {
    corruption_ = corruption;
    trailer_n_samples_ = trailer_n_samples;
    i_soi_ = i_soi;
    n_skipped_words_ = n_skipped_words;
  }

  /// set if the digis were filled from this event
  void setDigisFilled(bool filled) { digis_filled_ = filled; }

  /// add the next sample
  void addSample(const ECONDSampleInfo& sample) { samples_.push_back(sample); }

  /// frame corruption mask, bits from Corruption
  int getCorruption() const { return corruption_; }
  /// check a single frame corruption bit
  bool isCorrupt(Corruption c) const { return (corruption_ >> c) & 0b1; }
  /// number of samples in the ending trailer (-1 if missing)
  int getTrailerNumSamples() const { return trailer_n_samples_; }
  /// index of the sample of interest
  int getSOIIndex() const { return i_soi_; }
  /// number of words skipped searching for capture headers
  int getNumSkippedWords() const { return n_skipped_words_; }
  /// were all samples good so the digis were filled
  bool areDigisFilled() const { return digis_filled_; }
  /// samples in order of transmission
  const std::vector<ECONDSampleInfo>& getSamples() const { return samples_; }

  friend std::ostream& operator<<(std::ostream& o, const ECONDEventInfo& e);

 private:
  /// frame corruption mask
  int corruption_{0};
  /// number of samples in the trailer
  int trailer_n_samples_{-1};
  /// index of the sample of interest
  int i_soi_{0};
  /// words skipped while searching for capture headers
  int n_skipped_words_{0};
  /// digis were filled from this event
  bool digis_filled_{false};
  /// per-sample data
  std::vector<ECONDSampleInfo> samples_;

  ClassDef(ECONDEventInfo, 1);
};

}  // namespace ldmx

#endif  // RECON_EVENT_ECONDEVENTINFO_H_

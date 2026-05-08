#pragma once

#include "Packing/ECONDEventPacket.h"

namespace packing {

/**
 * Unpack an event that has potentially more than one sample
 * collected from a **single** ECOND.
 *
 * @see ECONDEventPacket for how a single sample from a single ECOND
 * is unpacked.
 *
 * @note This unpacking is not well tested and may change depending
 * on how the firmware/software progresses. The current draft was
 * written using TargetFiberless::read_event as a reference.
 */
class MultiSampleECONDEventPacket {
  /// handle to logging source
  enableLogging("MultiSampleECONDEventPacket");
  /// number of links connected to the ECOND
  int n_links_;

 public:
  /**
   * Corruption bits
   *
   * Index | Description
   * ------|------------
   *  0    | full packet header flag mismatch
  std::array<bool, 1> corruption;
   */
  /// index of the sample of interest (SOI)
  std::size_t i_soi;
  /// ID for this econ
  int econd_id;
  /// contributor ID
  int contrib_id;
  /// subsystem ID
  int subsys_id;
  /// timestamp from RoR
  uint64_t timestamp;
  /// get the sample of interest
  const ECONDEventPacket& soi() const;
  /// samples from ECOND stored in order of transmission
  std::vector<ECONDEventPacket> samples;
  /// constructor defining how many links are connected to this ECOND
  MultiSampleECONDEventPacket(int n_links);
  /// unpack the given data into this structure
  void from(std::span<uint32_t> data, bool expect_ldmx_ror_header = false);
  /// read into this structure from the input Reader
  Reader& read(Reader& r);

  /// header string if using to_csv
  static const std::string to_csv_header;

  /// write out all of the samples into the input CSV file
  void to_csv(std::ofstream& f) const;
};

}  // namespace pflib::packing

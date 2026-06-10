#pragma once
#ifndef PACKING_LDMXRORHEADER_H
#define PACKING_LDMXRORHEADER_H

#include <cstdint>
#include <unordered_map>
#include <vector>

#include "Packing/Utility/Reader.h"

namespace packing {

/**
 * the header that the LDMX DAQ Firmware block includes
 * in the output data stream at the beginning of each data frame
 *
 * This header is immediately preceeded by a RogueFrameHeader for
 * data frames. It is separate only becuase some frames written to
 * the file (mainly, the configuration stream) do not include this
 * header as well as the RogueFrameHeader.
 */
class LDMXRoRHeader {
 public:
  /// size of this header in bytes
  static const unsigned int SIZE = 16;
  /**
   * The subsystem ID numbers organized by subsystem name
   *
   * These ID numbers are compiled into the DAQ firmware.
   * Since the same firmware was used for both the ECal and HCal,
   * they both ended up having the same subsystem ID (5).
   *
   * The subsystem names are all lower case.
   */
  static const std::unordered_map<std::string, int> SUBSYSTEM_ID;

  /**
   * The contributor ID is a configurable parameter of the DAQ firmware.
   *
   * For the 2025 slice test at SLAC, there was only ever one contributor
   * per subsystem and so there is just one integer per subsystem here.
   * (-1 means that it was not set and can be ignored).
   *
   * In the future, there will be more than one contributor for some
   * subsystems.
   *
   * The subsystem names are all lower case.
   */
  static const std::unordered_map<std::string, int> CONTRIBUTOR_ID;

  bool valid() const { return valid_; }

  /**
   * Get the (subsystem, contributor) pair for the input subsystem name
   *
   * @note For the 2025 slice test at SLAC, there was only ever one contributor
   * per subsystem (-1 means that it was not set and can be ignored).
   * In the future, there will be more than one contributor for some
   * subsystems and so this function will need to be rewritten.
   *
   * @param[in] name subsystem name ('tdaq','ts','tracker','hcal','ecal')
   * @return 2-tuple of subsystem, contributor ID numbers. Returns the pair (-1,
   * -1) if the name is not found in our maps above.
   */
  static std::tuple<int, int> subsystem(const std::string& name);

  /// read the next LDMX RoR header into memory
  utility::Reader& read(utility::Reader& r);
  /// version of LDMX data (should be zero)
  uint8_t version() const { return version_; }
  /// ID number for subsystem originating data (compiled into firmware)
  uint8_t subsystem() const { return subsystem_; }
  /// ID number for contributor within subsystem (configured into firmware)
  uint8_t contributor() const { return contributor_; }
  /// get timestamp of this RoR
  uint64_t timestamp() const { return timestamp_; }

 private:
  /// version of LDMX data (should be zero)
  uint8_t version_;
  /// ID number for subsystem originating data (compiled into firmware)
  uint8_t subsystem_;
  /// ID number for contributor within subsystem (configured into firmware)
  uint8_t contributor_;
  /**
   * timestamp of this Readout-Request (RoR)
   *
   * It is some combination of pulseID and bunchCount,
   * but I'm unsure on how to decompose those at this
   * time.
   */
  uint64_t timestamp_;
  
  bool valid_{false};

};

}  // namespace packing

#endif
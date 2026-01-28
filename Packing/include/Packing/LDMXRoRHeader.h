#pragma once
#ifndef PACKING_LDMXRORHEADER_H
#define PACKING_LDMXRORHEADER_H

#include <cstdint>
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
};

}  // namespace packing

#endif

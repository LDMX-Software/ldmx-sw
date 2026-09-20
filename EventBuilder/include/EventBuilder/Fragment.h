// Fragment.h
#ifndef EVENTBUILDER_FRAGMENT_H
#define EVENTBUILDER_FRAGMENT_H

#include <cstdint>
#include <vector>

namespace eventbuilder {

// Define a simple CRC32 implementation for checksum
inline uint32_t crc32(const std::vector<uint8_t>& data) {
  uint32_t crc = 0xFFFFFFFF;
  for (uint8_t c : data) {
    crc ^= static_cast<uint8_t>(c);
    for (int i = 0; i < 8; ++i) {
      if (crc & 1) {
        crc = (crc >> 1) ^ 0xEDB88320;
      } else {
        crc >>= 1;
      }
    }
  }
  return ~crc;
}

enum class ContributorId { Channel, Module };

struct FragmentTrailer {
  uint32_t checksum_;  // For error detection
};

struct FragmentHeader {
  // Header Word 0
  uint64_t magic_number_ : 8;  // 0xA5
  uint64_t contributor_id_ : 8;
  uint64_t subsystem_id_ : 8;
  uint64_t version_ : 8;
  uint64_t padding_ : 32;  // Unused bits

  // Header Word 1
  uint64_t timestamp_;
};

// Represents a single data fragment
struct DataFragment {
  FragmentHeader header_;
  std::vector<uint8_t> payload_;  // Raw byte data from the readout
  FragmentTrailer trailer_;
};

}  // namespace eventbuilder

#endif

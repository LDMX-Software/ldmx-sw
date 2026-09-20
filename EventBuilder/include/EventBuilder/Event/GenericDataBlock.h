#ifndef GENERICDATABLOCK_H
#define GENERICDATABLOCK_H

#include <cstddef>
#include <cstdint>
#include <vector>

#include "EventBuilder/Fragment.h"

namespace eventbuilder {

struct GenericDataBlock {
  uint64_t subsystem_id_ = 0;
  uint64_t timestamp_ns_ = 0;
  std::vector<uint8_t> data_;  // raw encoded payload
  uint32_t checksum_ = 0;

  size_t size() const { return data_.size(); }
};

}  // namespace eventbuilder

#endif  // GENERICDATABLOCK_H

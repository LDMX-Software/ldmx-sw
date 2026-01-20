#pragma once
#ifndef PACKING_ROGUEFRAME_H
#define PACKING_ROGUEFRAME_H

#include <vector>
#include <cstdint>

#include "Packing/Utility/Reader.h"

namespace packing {

/**
 * parser for a Rogue frame written by a StreamWriter
 */
class RogueFrame {
 public:
  /// read the next rogue frame into memory
  utility::Reader& read(utility::Reader& r);
  /// get the buffer of data from disk
  const std::vector<uint8_t>& data() const { return data_; }
  /// get the channel this data was written to
  int channel() const { return channel_; }
 private:
  /// size of frame written by StreamWriter
  uint32_t size_;
  /// extra flags written by StreamWriter
  uint16_t flags_;
  /// error flags written by StreamWriter
  uint8_t error_;
  /// StreamWriter channel
  uint8_t channel_;
  /// buffer of bytes holding the data from disk for this channel
  std::vector<uint8_t> data_;
};

}

#endif

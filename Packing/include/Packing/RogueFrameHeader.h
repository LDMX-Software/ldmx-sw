#pragma once
#ifndef PACKING_ROGUEFRAMEHEADER_H
#define PACKING_ROGUEFRAMEHEADER_H

#include <vector>
#include <cstdint>

#include "Packing/Utility/Reader.h"

namespace packing {

/**
 * the header that the Rogue StreamWriter puts includes
 * at the beginning of each frame.
 */
class RogueFrameHeader {
 public:
  /// read the next rogue frame header into memory
  utility::Reader& read(utility::Reader& r);
  /// get the channel this data was written to
  int channel() const { return channel_; }
  /// get the size of the frame not including this header
  unsigned int size() const { return size_; }
  /// get the flags included in the header
  uint16_t flags() const { return flags_; }
  /// get specifc error flags included in the header
  uint8_t error() const { return error_; }
 private:
  /**
   * size of frame written by StreamWriter
   *
   * The number written on disk is 4 more than the
   * number we store here for later use because the
   * number on disk does not include the other 4 bytes
   * we parse into the other members below.
   */
  uint32_t size_;
  /// extra flags written by StreamWriter
  uint16_t flags_;
  /// error flags written by StreamWriter
  uint8_t error_;
  /// StreamWriter channel
  uint8_t channel_;
};

}

#endif

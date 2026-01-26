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
  /// get the trailer byte for checking
  uint8_t trailer() const { return trailer_; }
  /// check if this frame is probably a yaml dump
  bool probablyYaml() const {
    return trailer_ == 0x0a;
  }
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
  /**
   * last byte stored in frame
   *
   * we need to check the trailer byte as well since
   * many of the early runs took data with both the data
   * and yaml-dump of the config stored in the same channel
   *
   * The yaml-dump has a characteristic trailer byte 0x0a
   */
  uint8_t trailer_;
};

}

#endif

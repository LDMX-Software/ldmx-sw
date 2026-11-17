#include "Packing/ECONDEventPacket.h"

#include <algorithm>
#include <bitset>
#include <string>

#include "Packing/Utility/CRC.h"
#include "Packing/Utility/Hex.h"
#include "Packing/Utility/Mask.h"

using packing::utility::Hex;
using packing::utility::MASK;

namespace packing {

namespace {

/**
 * Construct the HGCROC sample word from the unpacked values
 *
 * Negative values were zero-suppressed by the ECON-D and are
 * represented by a value of zero.
 */
uint32_t packSample(bool tc, bool tp, int adc_tm1, int main_sample, int toa) {
  uint32_t msb_meas = (adc_tm1 < 0) ? 0 : adc_tm1;
  uint32_t lsb_meas = (toa < 0) ? 0 : toa;
  return (static_cast<uint32_t>(tc) << 31) + (static_cast<uint32_t>(tp) << 30) +
         ((msb_meas & MASK<10>) << 20) + ((main_sample & MASK<10>) << 10) +
         (lsb_meas & MASK<10>);
}

/// ECON-D Manual: MSB is HT match status, then EBO, then CRC
const std::array<const char*, 3> LINK_STAT_BIT_NAMES = {"CRC", "EBO", "HT"};

}  // namespace

std::size_t ECONDEventPacket::unpackLinkSubpacket(
    std::span<const uint32_t> data, ECONDLinkSubpacket& link) const {
  if (data.empty()) {
    ldmx_log(debug) << "no words left for link sub-packet";
    return 0;
  }
  ldmx_log(trace) << "link header " << Hex(data[0]);
  link.stat_ = ((data[0] >> 29) & MASK<3>);
  if (link.badStat()) {
    std::bitset<3> bits = link.stat_;
    std::string names;
    for (std::size_t i{0}; i < 3; i++) {
      if (!bits.test(i)) names += std::string(" ") + LINK_STAT_BIT_NAMES[i];
    }
    ldmx_log(debug) << "bad link Stat bits 0b" << bits << " -> [" << names
                    << " ] mismatched";
  }
  link.hamming_ = ((data[0] >> 26) & MASK<3>);
  link.is_empty_ = ((data[0] >> 25) & MASK<1>) == 1;
  link.adc_cm0_ = ((data[0] >> 15) & MASK<10>);
  link.adc_cm1_ = ((data[0] >> 5) & MASK<10>);
  ldmx_log(trace) << "    is_empty=" << link.is_empty_
                  << " CM0=" << link.adc_cm0_ << " CM1=" << link.adc_cm1_;

  if (link.is_empty_) {
    // F=1: single-word header, no channel map or channel data
    // E=1 means unmasked Stat error bits caused the suppression
    link.error_caused_empty_ = ((data[0] >> 4) & MASK<1>) == 1;
    ldmx_log(trace) << "is empty, E=" << link.error_caused_empty_;
    return 1;
  }

  if (data.size() < 2) {
    ldmx_log(debug) << "link sub-packet missing channel map";
    return 0;
  }
  ldmx_log(trace) << "chan map lower 32 " << Hex(data[1]);
  link.channel_map_ =
      ((data[0] & MASK<5>) << 32) | static_cast<uint64_t>(data[1]);
  ldmx_log(trace) << "is not empty, channel_map="
                  << std::bitset<37>(link.channel_map_);

  // two header words (including channel map)
  std::size_t length{2};

  int bits_left_in_current_word{32};
  for (std::size_t i_chan{0}; i_chan < 37; i_chan++) {
    // calib is the 19th channel in the sub-packet
    uint32_t* ch = &(link.calib_);
    if (i_chan < 18) {
      ch = &(link.channels_[i_chan]);
    } else if (i_chan > 18) {
      ch = &(link.channels_[i_chan - 1]);
    }

    if (passthrough_) {
      // channels are transparently copied from the link
      if (length >= data.size()) {
        ldmx_log(debug) << "pass through sub-packet ran out of words at ch "
                        << i_chan;
        return 0;
      }
      *ch = data[length];
      length++;
      continue;
    }

    // skip channels that are not passed
    if (((link.channel_map_ >> i_chan) & 1) == 0) {
      *ch = 0;
      continue;
    }

    if (length >= data.size()) {
      ldmx_log(debug) << "sub-packet ran out of words at ch " << i_chan;
      return 0;
    }

    // the code cannot span words, the format is byte aligned
    auto code{(data[length] >> (bits_left_in_current_word - 4)) & MASK<4>};
    // direct transcription of the table in Fig 20 of the ECOND Spec
    int nbits{32}, extra{0};
    bool has_adctm1{true}, has_toa{true};
    bool tc{false}, tp{false};
    if (code == 0b0000) {
      // TOA ZS
      has_toa = false;
      nbits = 24;
    } else if (code == 0b0001) {
      // ADC(-1) and TOA ZS
      has_adctm1 = false;
      has_toa = false;
      extra = 2;
      nbits = 16;
    } else if (code == 0b0010) {
      // No ZS or ...
      tp = true;
      has_toa = false;
      nbits = 24;
    } else if (code == 0b0011) {
      // ADC(-1) ZS
      has_adctm1 = false;
      nbits = 24;
    } else if ((code >> 2) == 0b01) {
      // full pass ZS in ADC mode
    } else if ((code >> 2) == 0b11) {
      // pass ZS because in TOT mode
      tc = true;
      tp = true;
    } else {
      // 0b10: known invalid
      ldmx_log(debug) << "ECOND eRx known invalid code "
                      << std::bitset<2>(code >> 2);
      tc = true;
    }

    // get the channel data from the current and perhaps the next word
    uint64_t chan_data{0};
    if (bits_left_in_current_word < nbits) {
      int runon_length = nbits - bits_left_in_current_word;
      if (length + 1 >= data.size()) {
        ldmx_log(debug) << "sub-packet ran out of words at ch " << i_chan;
        return 0;
      }
      chan_data = ((data[length] & ((1ul << bits_left_in_current_word) - 1ul))
                   << runon_length);
      length++;
      bits_left_in_current_word = 32;
      chan_data +=
          ((data[length] >> (bits_left_in_current_word - runon_length)) &
           ((1ul << runon_length) - 1ul));
      bits_left_in_current_word -= runon_length;
    } else {
      chan_data = ((data[length] >> (bits_left_in_current_word - nbits)) &
                   ((1ul << nbits) - 1ul));
      bits_left_in_current_word -= nbits;
    }

    // measurements that do not exist are negative
    int adc_tm1{-1}, toa{-1};
    chan_data >>= extra;
    if (has_toa) {
      toa = (chan_data & MASK<10>);
      chan_data >>= 10;
    }
    int main_sample = (chan_data & MASK<10>);
    chan_data >>= 10;
    if (has_adctm1) {
      adc_tm1 = (chan_data & MASK<10>);
    }
    ldmx_log(trace) << "    ch " << i_chan << " code=" << std::bitset<4>(code)
                    << " ADC(t-1)=" << adc_tm1 << " ADC/TOT=" << main_sample
                    << " TOA=" << toa;

    *ch = packSample(tc, tp, adc_tm1, main_sample, toa);

    if (bits_left_in_current_word == 0) {
      length++;
      bits_left_in_current_word = 32;
    }
  }

  // sub-packets are 32b-word-aligned
  if (bits_left_in_current_word < 32) {
    length++;
  }

  if (length > data.size()) {
    ldmx_log(debug) << "sub-packet length " << length << " exceeds "
                    << data.size() << " remaining words";
    return 0;
  }

  ldmx_log(trace) << "eRx final length " << length;
  return length;
}

ECONDEventPacket::ECONDEventPacket(std::size_t n_links) : links_(n_links) {}

bool ECONDEventPacket::isCorrupt() const {
  return std::any_of(corruption_.begin(), corruption_.end(),
                     [](bool c) { return c; });
}

bool ECONDEventPacket::from(std::span<const uint32_t> data) {
  corruption_.fill(false);
  std::fill(links_.begin(), links_.end(), ECONDLinkSubpacket{});
  payload_length_ = 0;

  if (data.size() < 2) {
    ldmx_log(debug) << "packet of " << data.size()
                    << " words is too short for the two header words";
    corruption_[BAD_LENGTH] = true;
    if (data.empty()) return false;
  }

  ldmx_log(trace) << "econd header one: " << Hex(data[0]);
  header_marker_ = ((data[0] >> 23) & MASK<9>);
  corruption_[BAD_HEADER_MARKER] = (header_marker_ != HEADER_MARKER_SPEC) and
                                   (header_marker_ != HEADER_MARKER_CMS);
  if (corruption_[BAD_HEADER_MARKER]) {
    // most likely not an ECON-D header at all
    ldmx_log(debug) << "Bad header marker from ECOND 0x" << std::hex
                    << (header_marker_ >> 1) << " + 0b"
                    << (header_marker_ & 0b1) << std::dec << " in word "
                    << Hex(data[0]);
    return false;
  }
  if (corruption_[BAD_LENGTH]) return false;

  payload_length_ = ((data[0] >> 14) & MASK<9>);
  passthrough_ = ((data[0] >> 13) & MASK<1>) == 1;
  expected_ = ((data[0] >> 12) & MASK<1>) == 1;
  ht_ = ((data[0] >> 10) & MASK<2>);
  ebo_ = ((data[0] >> 8) & MASK<2>);
  match_ = ((data[0] >> 7) & MASK<1>) == 1;
  truncated_ = ((data[0] >> 6) & MASK<1>) == 1;
  hamming_ = (data[0] & MASK<6>);

  ldmx_log(trace) << "econd header two: " << Hex(data[1]);
  bx_ = ((data[1] >> 20) & MASK<12>);
  l1a_ = ((data[1] >> 14) & MASK<6>);
  orbit_ = ((data[1] >> 11) & MASK<3>);
  subpacket_error_ = ((data[1] >> 10) & MASK<1>) == 1;
  reset_request_ = ((data[1] >> 8) & MASK<2>);
  crc8_ = (data[1] & MASK<8>);
  ldmx_log(trace) << "    length=" << payload_length_ << " P=" << passthrough_
                  << " BX=" << bx_ << " L1A=" << l1a_ << " Orb=" << orbit_;

  corruption_[BAD_LENGTH] = (data.size() != (payload_length_ + 2));
  if (corruption_[BAD_LENGTH]) {
    ldmx_log(debug) << "stored payload length " << payload_length_
                    << " does not match the packet length (" << data.size()
                    << " - 2)";
    if (data.size() < (payload_length_ + 2)) return false;
  }

  // 8b CRC over both header words without the Hamming and the CRC
  uint64_t header_crc_base = (data[0] >> 6);
  header_crc_base <<= 30;
  header_crc_base |= (data[1] >> 8);
  uint8_t header_crc_val = utility::econdCrc8(header_crc_base);
  corruption_[BAD_HEADER_CRC] = (header_crc_val != crc8_);
  if (corruption_[BAD_HEADER_CRC]) {
    ldmx_log(debug) << "Event header 8b CRC does not match transmitted value: "
                    << std::bitset<8>(header_crc_val)
                    << " != " << std::bitset<8>(crc8_);
  }

  if (isEmpty()) {
    ldmx_log(trace) << "empty packet, no link sub-packets";
    return not isCorrupt();
  }

  // the last payload word is the CRC over all link sub-packets
  auto payload{data.subspan(2, payload_length_)};
  auto subpackets{payload.first(payload_length_ - 1)};
  uint32_t crc_val = utility::crc32(subpackets);
  uint32_t target = payload.back();
  corruption_[BAD_CRC32] = (crc_val != target);
  if (corruption_[BAD_CRC32]) {
    ldmx_log(debug)
        << "CRC over all link sub-packets does not match transmitted value "
        << Hex(crc_val) << " != " << Hex(target);
  }

  if (corruption_[BAD_HEADER_CRC]) return false;

  if (truncated_) {
    ldmx_log(debug) << "Buffer control detected insufficient space so it did "
                       "not transfer link packets.";
    corruption_[TRUNCATED] = true;
    return false;
  }

  std::size_t offset{0};
  for (auto& link : links_) {
    auto length = unpackLinkSubpacket(subpackets.subspan(offset), link);
    if (length == 0) {
      corruption_[BAD_PAYLOAD] = true;
      return false;
    }
    offset += length;
  }

  if (offset != subpackets.size()) {
    ldmx_log(debug) << "link sub-packets end at " << offset
                    << " but the CRC word is at " << subpackets.size();
    corruption_[BAD_PAYLOAD] = true;
  }

  return not isCorrupt();
}

}  // namespace packing

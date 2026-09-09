#ifndef TRACKING_RECO_SISTRIPCHANNELMAP_H_
#define TRACKING_RECO_SISTRIPCHANNELMAP_H_

#include <cstdint>
#include <cstdio>
#include <string>

namespace tracking::reco {

/**
 * Central definition of the tracker silicon-strip channel mapping.
 *
 * Every electronics-id <-> physical-strip conversion (and the keys used to
 * group/lookup channels) lives here so the layout only has to change in one
 * place.  All functions are header-only constexpr/inline helpers; there is no
 * per-instance state.
 */
namespace channelmap {

/// Number of readout channels per APV25 chip.
inline constexpr int K_CHANNELS_PER_APV = 128;
/// Number of APV25 chips per hybrid.
inline constexpr int K_APVS_PER_HYBRID = 5;
/// Number of physical strips per hybrid (kApvsPerHybrid * kChannelsPerApv).
inline constexpr int K_CHANNELS_PER_HYBRID =
    K_APVS_PER_HYBRID * K_CHANNELS_PER_APV;
/// Number of ADC samples the APV25 reads out per channel per trigger.
inline constexpr int K_SAMPLES_PER_APV_TRIGGER = 3;

/**
 * Convert an (APV id, APV channel) pair into the physical strip number
 * (pchannel) within a hybrid, in [0, kChannelsPerHybrid).
 *
 * The APV readout order is reversed relative to the physical strip order, and
 * the APVs are laid out back-to-front along the hybrid, hence the double
 * reversal.
 */
inline constexpr int16_t pchannel(uint8_t apv_id, uint8_t channel) {
  return static_cast<int16_t>(
      (K_CHANNELS_PER_HYBRID - 1) -
      (apv_id * K_CHANNELS_PER_APV + (K_CHANNELS_PER_APV - 1) - channel));
}

/**
 * Inverse of pchannel(): recover the (APV id, APV channel) pair from a physical
 * strip number.  Needed because SiStripWaveform stores only the pchannel, but
 * the pedestal table is keyed by the full electronics address.
 *
 * Round-trips exactly with pchannel() for all valid (apv, channel):
 *   apvChannelFromPchannel(pchannel(a, c), a', c') gives a' == a, c' == c.
 */
inline constexpr void apvChannelFromPchannel(int16_t pchannel, uint8_t& apv_id,
                                             uint8_t& channel) {
  const int x = (K_CHANNELS_PER_HYBRID - 1) - pchannel;
  apv_id = static_cast<uint8_t>(x / K_CHANNELS_PER_APV);
  channel =
      static_cast<uint8_t>((K_CHANNELS_PER_APV - 1) - (x % K_CHANNELS_PER_APV));
}

/**
 * Map a physical strip number (pchannel) onto a sensor strip index using a
 * DAQ-map transform: an optional readout reversal plus a per-sensor offset.
 *
 * @param pchannel    physical strip within the hybrid, [0, kChannelsPerHybrid).
 * @param n_strips    number of bonded strips on the sensor.
 * @param first_strip sensor strip index that pchannel 0 (or the reversed end)
 *                    maps to.
 * @param reversed    true if the hybrid reads the sensor in descending order.
 */
inline constexpr int stripId(int16_t pchannel, int n_strips, int first_strip,
                             bool reversed) {
  return reversed ? first_strip + n_strips - 1 - pchannel
                  : first_strip + pchannel;
}

/// Build the per-channel pedestal-map key "feb:hybrid:apv:channel".
inline std::string channelKey(uint8_t feb, uint8_t hybrid, uint8_t apv,
                              uint8_t channel) {
  return std::to_string(feb) + ":" + std::to_string(hybrid) + ":" +
         std::to_string(apv) + ":" + std::to_string(channel);
}

/// Pack the full electronics address (feb, hybrid, apv, channel) into a single
/// 32-bit id, used as the row key of the pedestal conditions table.
inline constexpr uint32_t channelId(uint8_t feb, uint8_t hybrid, uint8_t apv,
                                    uint8_t channel) {
  return (static_cast<uint32_t>(feb) << 24) |
         (static_cast<uint32_t>(hybrid) << 16) |
         (static_cast<uint32_t>(apv) << 8) | static_cast<uint32_t>(channel);
}

/// Parse a "feb:hybrid:apv:channel" key (as produced by channelKey()) back into
/// its electronics fields.  Returns false if the string is malformed.
inline bool parseChannelKey(const std::string& key, uint8_t& feb,
                            uint8_t& hybrid, uint8_t& apv, uint8_t& channel) {
  unsigned f, h, a, c;
  if (std::sscanf(key.c_str(), "%u:%u:%u:%u", &f, &h, &a, &c) != 4)
    return false;
  feb = static_cast<uint8_t>(f);
  hybrid = static_cast<uint8_t>(h);
  apv = static_cast<uint8_t>(a);
  channel = static_cast<uint8_t>(c);
  return true;
}

/// Pack (feb, hybrid, pchannel) into a single 32-bit waveform-grouping key.
inline constexpr uint32_t groupKey(uint8_t feb, uint8_t hybrid,
                                   int16_t pchannel) {
  return (static_cast<uint32_t>(feb) << 24) |
         (static_cast<uint32_t>(hybrid) << 16) |
         static_cast<uint32_t>(static_cast<uint16_t>(pchannel));
}

/// Extract the pchannel back out of a key produced by groupKey().
inline constexpr int16_t pchannelFromGroupKey(uint32_t key) {
  return static_cast<int16_t>(key & 0xFFFF);
}

}  // namespace channelmap
}  // namespace tracking::reco

#endif  // TRACKING_RECO_SISTRIPCHANNELMAP_H_

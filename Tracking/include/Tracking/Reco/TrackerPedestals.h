#ifndef TRACKING_RECO_TRACKERPEDESTALS_H_
#define TRACKING_RECO_TRACKERPEDESTALS_H_

#include <array>
#include <cstdint>
#include <numeric>
#include <string>
#include <unordered_map>

#include "Framework/ConditionsObject.h"
#include "Tracking/Reco/SiStripChannelMap.h"

namespace tracking::reco {

/**
 * Per-channel tracker pedestal conditions: the per-sample pedestal mean and RMS
 * noise for every silicon-strip readout channel.
 *
 * This is a conditions object (a *service*), not event data: pedestals and
 * noise are calibration constants delivered on demand via getCondition<>(), the
 * same way Ecal/Hcal deliver their HGCROC pedestals/gains.  Consumers
 * (PedestalSubtractor, SiStripWaveformBuilder, ...) look up values by
 * electronics address; nothing is persisted into the hit objects.
 *
 * The object is filled by TrackerPedestalProvider, which reads the pedestal
 * file produced by PedestalCalculator.
 */
class TrackerPedestals : public framework::ConditionsObject {
 public:
  /// Name of the conditions object (must match the python registration).
  static const std::string CONDITIONS_NAME;

  /// Per-channel pedestal mean and RMS noise for the three APV25 samples.
  struct Channel {
    std::array<float, channelmap::K_SAMPLES_PER_APV_TRIGGER> mean_{};
    std::array<float, channelmap::K_SAMPLES_PER_APV_TRIGGER> noise_{};
  };

  TrackerPedestals() : framework::ConditionsObject(CONDITIONS_NAME) {}

  /// Insert the pedestal for one electronics channel.
  void add(uint8_t feb, uint8_t hybrid, uint8_t apv, uint8_t channel,
           const Channel& ped) {
    channels_[channelmap::channelId(feb, hybrid, apv, channel)] = ped;
  }

  /// Look up a channel; returns nullptr if the channel is not in the table.
  const Channel* find(uint8_t feb, uint8_t hybrid, uint8_t apv,
                      uint8_t channel) const {
    auto it = channels_.find(channelmap::channelId(feb, hybrid, apv, channel));
    return it == channels_.end() ? nullptr : &it->second;
  }

  /// Scalar per-channel noise (mean of the three per-sample noises); 0 if the
  /// channel is absent.
  float noise(uint8_t feb, uint8_t hybrid, uint8_t apv, uint8_t channel) const {
    const Channel* c = find(feb, hybrid, apv, channel);
    if (!c) return 0.f;
    const float sum =
        std::accumulate(c->noise_.begin(), c->noise_.end(), 0.f);
    return sum / static_cast<float>(c->noise_.size());
  }

  /// Number of channels in the table.
  std::size_t size() const { return channels_.size(); }

 private:
  std::unordered_map<uint32_t, Channel> channels_;
};

}  // namespace tracking::reco

#endif  // TRACKING_RECO_TRACKERPEDESTALS_H_

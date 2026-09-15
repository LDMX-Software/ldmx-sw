#include "Tracking/Reco/SiStripWaveformBuilder.h"

#include <algorithm>
#include <map>

#include "Tracking/Event/RawSiStripHit.h"
#include "Tracking/Reco/SiStripChannelMap.h"
#include "Tracking/Reco/TrackerPedestals.h"

namespace tracking::reco {

void SiStripWaveformBuilder::configure(framework::config::Parameters& ps) {
  input_collection_ =
      ps.get<std::string>("input_collection", input_collection_);
  input_pass_name_ = ps.get<std::string>("input_pass_name", input_pass_name_);
  output_collection_ =
      ps.get<std::string>("output_collection", output_collection_);
  high_threshold_ = ps.get<double>("high_threshold", high_threshold_);
  min_high_samples_ = ps.get<int>("min_high_samples", min_high_samples_);
  low_threshold_ = ps.get<double>("low_threshold", low_threshold_);
  min_consecutive_low_ =
      ps.get<int>("min_consecutive_low", min_consecutive_low_);
  n_triggers_ = ps.get<int>("n_triggers", n_triggers_);
}

void SiStripWaveformBuilder::produce(framework::Event& event) {
  const auto& peds =
      getCondition<TrackerPedestals>(TrackerPedestals::CONDITIONS_NAME);

  const auto& hits = event.getCollection<ldmx::RawSiStripHit>(input_collection_,
                                                              input_pass_name_);

  // Key: encodes (feb, hybrid, pchannel) uniquely.
  // Value: vector of (apv_trigger, samples) pairs.
  struct TriggerSamples {
    uint16_t apv_trigger_;
    std::vector<short> samples_;
  };
  struct ChannelInfo {
    float noise_{0};
    uint8_t hybrid_id_{0};
    uint8_t feb_id_{0};
    std::vector<TriggerSamples> triggers_;
  };

  // Use a map keyed by (feb, hybrid, pchannel) for grouping.
  std::map<uint32_t, ChannelInfo> channel_map;

  for (const auto& hit : hits) {
    const float noise = peds.noise(hit.getFebId(), hit.getHybridId(),
                                   hit.getApvId(), hit.getChannel());
    if (noise <= 0) continue;

    const int16_t pchannel =
        channelmap::pchannel(hit.getApvId(), hit.getChannel());
    uint32_t key =
        channelmap::groupKey(hit.getFebId(), hit.getHybridId(), pchannel);

    auto& ch = channel_map[key];
    ch.noise_ = noise;
    ch.hybrid_id_ = hit.getHybridId();
    ch.feb_id_ = hit.getFebId();
    ch.triggers_.push_back({hit.getApvTrigger(), hit.getSamples()});
  }

  std::vector<ldmx::SiStripWaveform> waveforms;

  for (auto& [key, ch] : channel_map) {
    // Sort triggers by apv_trigger index.
    std::sort(ch.triggers_.begin(), ch.triggers_.end(),
              [](const TriggerSamples& a, const TriggerSamples& b) {
                return a.apv_trigger_ < b.apv_trigger_;
              });

    // Assemble full waveform: [s0_t0, s1_t0, s2_t0, s0_t1, ...].
    std::vector<short> samples;
    samples.reserve(ch.triggers_.size() *
                    channelmap::K_SAMPLES_PER_APV_TRIGGER);
    for (const auto& trig : ch.triggers_) {
      for (short s : trig.samples_) samples.push_back(s);
    }

    if (samples.empty()) continue;

    // Condition 1: at least min_high_samples_ samples exceed high_threshold_.
    int n_high = 0;
    for (short s : samples)
      if (static_cast<float>(s) / ch.noise_ > high_threshold_) ++n_high;
    if (n_high < min_high_samples_) continue;

    // Condition 2: at least min_consecutive_low_ consecutive samples exceed
    // low_threshold_.
    int max_streak = 0, cur_streak = 0;
    for (short s : samples) {
      if (static_cast<float>(s) / ch.noise_ > low_threshold_) {
        max_streak = std::max(max_streak, ++cur_streak);
      } else {
        cur_streak = 0;
      }
    }
    if (max_streak < min_consecutive_low_) continue;

    int16_t pchannel = channelmap::pchannelFromGroupKey(key);
    uint8_t n_trig = static_cast<uint8_t>(
        std::min(static_cast<int>(ch.triggers_.size()), 255));

    waveforms.emplace_back(std::move(samples), pchannel, ch.hybrid_id_,
                           ch.feb_id_, n_trig);
  }

  ldmx_log(debug) << "Built " << waveforms.size()
                  << " waveforms (>=" << min_high_samples_ << " samples @"
                  << high_threshold_ << "s, streak>=" << min_consecutive_low_
                  << " @" << low_threshold_ << "s) from " << hits.size()
                  << " hits";

  for (const auto& wf : waveforms) {
    ldmx_log(trace) << wf;
  }

  event.add(output_collection_, waveforms);
}

}  // namespace tracking::reco

DECLARE_PRODUCER(tracking::reco::SiStripWaveformBuilder)

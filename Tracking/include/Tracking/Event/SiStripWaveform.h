#ifndef TRACKING_EVENT_SISTRIPWAVEFORM_H_
#define TRACKING_EVENT_SISTRIPWAVEFORM_H_

#include <algorithm>
#include <iostream>
#include <vector>

#include "TObject.h"

namespace ldmx {

/**
 * Full multi-trigger waveform for one silicon strip channel.
 *
 * Assembled by SiStripWaveformBuilder from the per-trigger pedestal-subtracted
 * RawSiStripHit objects for a single (feb, hybrid, pchannel).  Samples are stored in
 * trigger order: [s0_t0, s1_t0, s2_t0, s0_t1, s1_t1, s2_t1, ...], so
 * the total size is n_triggers * 3.
 *
 * This is the natural input for waveform fitting (APV25 CR-RC pulse
 * shape → hit time with ~2.5 ns resolution) and any per-channel
 * waveform corrections before clustering.
 */
class SiStripWaveform {
 public:
  SiStripWaveform() = default;

  SiStripWaveform(std::vector<short> samples, float noise, int16_t pchannel,
                  uint8_t hybrid_id, uint8_t feb_id, uint8_t n_triggers);

  virtual ~SiStripWaveform() {}

  void clear();

  const std::vector<short>& getSamples()   const { return samples_; }
  float                     getNoise()     const { return noise_; }
  int16_t                   getPchannel()  const { return pchannel_; }
  uint8_t                   getHybridId()  const { return hybrid_id_; }
  uint8_t                   getFebId()     const { return feb_id_; }
  uint8_t                   getNTriggers() const { return n_triggers_; }

  /// Sample at trigger index t (0-based), APV sample s (0-2).
  short getSample(uint8_t t, uint8_t s) const {
    return samples_[t * 3 + s];
  }

  /// Peak significance: max(samples) / noise across the full waveform.
  float peakSigma() const {
    if (noise_ <= 0 || samples_.empty()) return 0.f;
    auto mx = *std::max_element(samples_.begin(), samples_.end());
    return static_cast<float>(mx) / noise_;
  }

  /// Trigger index (0-based) of the sample with the maximum ADC value.
  uint8_t peakTrigger() const {
    if (samples_.empty()) return 0;
    auto it = std::max_element(samples_.begin(), samples_.end());
    return static_cast<uint8_t>(std::distance(samples_.begin(), it) / 3);
  }

  friend std::ostream& operator<<(std::ostream& o, const SiStripWaveform& w);

 protected:
  std::vector<short> samples_;   ///< n_triggers * 3 pedestal-subtracted ADC samples
  float   noise_{0};             ///< per-channel RMS noise from pedestal run
  int16_t pchannel_{0};          ///< physical strip number within hybrid [0, 639]
  uint8_t hybrid_id_{0};
  uint8_t feb_id_{0};
  uint8_t n_triggers_{0};        ///< number of APV triggers assembled

  ClassDef(SiStripWaveform, 1);
};

}  // namespace ldmx

#endif  // TRACKING_EVENT_SISTRIPWAVEFORM_H_

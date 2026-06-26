#include "Tracking/Event/SiStripWaveform.h"

#include <algorithm>
#include <iomanip>
#include <string>

namespace ldmx {

SiStripWaveform::SiStripWaveform(std::vector<short> samples, int16_t pchannel,
                                 uint8_t hybrid_id, uint8_t feb_id,
                                 uint8_t n_triggers)
    : samples_(std::move(samples)),
      pchannel_(pchannel),
      hybrid_id_(hybrid_id),
      feb_id_(feb_id),
      n_triggers_(n_triggers) {}

void SiStripWaveform::clear() {
  samples_.clear();
  pchannel_   = 0;
  hybrid_id_  = 0;
  feb_id_     = 0;
  n_triggers_ = 0;
}

std::ostream& operator<<(std::ostream& output, const SiStripWaveform& w) {
  // Header.
  output << "[ SiStripWaveform ] FEB=" << static_cast<int>(w.feb_id_)
         << " Hybrid=" << static_cast<int>(w.hybrid_id_)
         << " PCh=" << w.pchannel_
         << " NTrig=" << static_cast<int>(w.n_triggers_)
         << " PeakAmp=" << w.peakAmplitude()
         << "\n";

  if (w.samples_.empty()) return output;

  // Determine scale — always include zero so the baseline is visible.
  short s_min = *std::min_element(w.samples_.begin(), w.samples_.end());
  short s_max = *std::max_element(w.samples_.begin(), w.samples_.end());
  s_min = std::min(s_min, static_cast<short>(0));
  s_max = std::max(s_max, static_cast<short>(0));

  const int bar_width = 50;
  const float range = static_cast<float>(s_max - s_min);

  output << "  Idx    ADC  |\n";

  for (int i = 0; i < static_cast<int>(w.samples_.size()); ++i) {
    short val = w.samples_[i];
    int t = i / 3, s = i % 3;

    const std::string label =
        "T" + std::to_string(t) + "." + std::to_string(s);

    // Column positions for zero and this sample.
    int zero_pos = (range > 0)
                       ? static_cast<int>((0.f - s_min) / range * (bar_width - 1))
                       : bar_width / 2;
    int val_pos  = (range > 0)
                       ? static_cast<int>((static_cast<float>(val) - s_min)
                                          / range * (bar_width - 1))
                       : zero_pos;
    val_pos = std::max(0, std::min(bar_width - 1, val_pos));

    std::string bar(bar_width, ' ');
    // Fill between zero and sample.
    int lo = std::min(zero_pos, val_pos);
    int hi = std::max(zero_pos, val_pos);
    for (int k = lo; k <= hi; ++k)
      bar[k] = (val >= 0) ? '+' : '-';
    // Tip marker.
    bar[val_pos] = (val > 0) ? '>' : (val < 0) ? '<' : '|';
    // Zero marker (only when sample is non-zero).
    if (val != 0 && bar[zero_pos] == ' ') bar[zero_pos] = '|';

    output << "  " << std::left << std::setw(4) << label << "  " << std::right
           << std::setw(5) << static_cast<int>(val) << "  |" << bar << "|\n";
  }
  return output;
}

}  // namespace ldmx

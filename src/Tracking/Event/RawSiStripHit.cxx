
#include "Tracking/Event/RawSiStripHit.h"

namespace ldmx {

RawSiStripHit::RawSiStripHit(std::vector<short> samples, long time)
    : samples_(samples), time_(time) {}

void RawSiStripHit::Clear() {
  samples_.clear();
  time_ = 0;
}

std::ostream &operator<<(std::ostream &output, const RawSiStripHit &hit) {
  output << "[ RawSiStripHit ]: Samples: { ";
  for (auto isample{0}; isample < (hit.samples_.size() - 1); ++isample)
    output << hit.samples_[isample] << ", ";
  output << hit.samples_[hit.samples_.size() - 1] << " } "
         << "Time: " << hit.time_ << std::endl;

  return output;
}

} // namespace ldmx

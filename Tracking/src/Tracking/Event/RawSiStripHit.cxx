
#include "Tracking/Event/RawSiStripHit.h"

namespace ldmx {

RawSiStripHit::RawSiStripHit(int layer_id, int strip_id,
                              std::vector<short> samples, long time)
    : layer_id_(layer_id), strip_id_(strip_id), samples_(samples), time_(time) {}

void RawSiStripHit::clear() {
  layer_id_ = -1;
  strip_id_ = -1;
  samples_.clear();
  time_ = 0;
}

std::ostream &operator<<(std::ostream &output, const RawSiStripHit &hit) {
  output << "[ RawSiStripHit ]: layer=" << hit.layer_id_
         << " strip=" << hit.strip_id_ << " Samples: { ";
  for (auto isample{0}; isample < (int)(hit.samples_.size() - 1); ++isample)
    output << hit.samples_[isample] << ", ";
  output << hit.samples_[hit.samples_.size() - 1] << " } "
         << "Time: " << hit.time_ << std::endl;

  return output;
}

}  // namespace ldmx

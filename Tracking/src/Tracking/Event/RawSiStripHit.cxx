
#include "Tracking/Event/RawSiStripHit.h"

ClassImp(ldmx::RawSiStripHit);

namespace ldmx {

RawSiStripHit::RawSiStripHit(uint8_t channel, std::vector<short> samples,
                             long time)
    : SiStripHit(samples, time), channel_(channel) {}

void RawSiStripHit::clear() {
  clearBase();
  channel_ = 0;
  apv_id_ = 0;
  hybrid_id_ = 0;
  feb_id_ = 0;
  apv_trigger_ = 0;
  read_error_ = 0;
  head_ = 0;
  tail_ = 0;
  filter_ = 0;
}

std::ostream &operator<<(std::ostream &output, const RawSiStripHit &hit) {
  output << "[ RawSiStripHit ]: Samples: { ";
  for (auto isample{0}; isample < (hit.samples_.size() - 1); ++isample)
    output << hit.samples_[isample] << ", ";
  output << hit.samples_[hit.samples_.size() - 1] << " } "
         << "Time: " << hit.time_
         << " Ch: " << static_cast<int>(hit.channel_)
         << " APV: " << static_cast<int>(hit.apv_id_)
         << " Hybrid: " << static_cast<int>(hit.hybrid_id_)
         << " FEB: " << static_cast<int>(hit.feb_id_)
         << " Trig: " << hit.apv_trigger_
         << " Err: " << static_cast<int>(hit.read_error_)
         << std::endl;

  return output;
}

}  // namespace ldmx

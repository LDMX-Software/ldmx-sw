
#include "Tracking/Event/RawSiStripHit.h"

namespace ldmx {

RawSiStripHit::RawSiStripHit(std::vector<short> samples, long time)
    : samples_(samples), time_(time) {}

RawSiStripHit::RawSiStripHit(std::vector<short> samples, long time,
                              uint8_t channel, uint8_t apv_id,
                              uint8_t hybrid_id, uint8_t feb_id,
                              uint16_t apv_trigger, uint8_t read_error,
                              uint8_t head, uint8_t tail, uint8_t filter)
    : samples_(samples),
      time_(time),
      channel_(channel),
      apv_id_(apv_id),
      hybrid_id_(hybrid_id),
      feb_id_(feb_id),
      apv_trigger_(apv_trigger),
      read_error_(read_error),
      head_(head),
      tail_(tail),
      filter_(filter) {}

void RawSiStripHit::clear() {
  samples_.clear();
  time_ = 0;
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

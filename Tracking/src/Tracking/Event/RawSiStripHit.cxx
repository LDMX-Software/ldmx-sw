
#include "Tracking/Event/RawSiStripHit.h"

namespace ldmx {

RawSiStripHit::RawSiStripHit(int layer_id, int strip_id,
                              std::vector<short> samples, long time,
                              int track_id, int pdg_id, int sim_hit_id,
                              float edep)
    : layer_id_(layer_id), strip_id_(strip_id), samples_(samples), time_(time),
      track_id_(track_id), pdg_id_(pdg_id), sim_hit_id_(sim_hit_id),
      edep_(edep) {}

void RawSiStripHit::clear() {
  layer_id_ = -1;
  strip_id_ = -1;
  samples_.clear();
  time_ = 0;
  track_id_ = -1;
  pdg_id_ = 0;
  sim_hit_id_ = -1;
  edep_ = 0.f;
}

std::ostream &operator<<(std::ostream &output, const RawSiStripHit &hit) {
  output << "[ RawSiStripHit ]: layer=" << hit.layer_id_
         << " strip=" << hit.strip_id_ << " Samples: { ";
  for (auto isample{0}; isample < (int)(hit.samples_.size() - 1); ++isample)
    output << hit.samples_[isample] << ", ";
  output << hit.samples_[hit.samples_.size() - 1] << " } "
         << "Time: " << hit.time_
         << " track_id=" << hit.track_id_
         << " pdg_id=" << hit.pdg_id_
         << " sim_hit_id=" << hit.sim_hit_id_
         << " edep=" << hit.edep_ << " MeV" << std::endl;

  return output;
}

}  // namespace ldmx

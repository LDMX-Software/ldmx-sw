
#include "Tracking/Event/SimSiStripHit.h"

ClassImp(ldmx::SimSiStripHit);

namespace ldmx {

SimSiStripHit::SimSiStripHit(int layer_id, int strip_id,
                             std::vector<short> samples, long time,
                             int track_id, int pdg_id, int sim_hit_id,
                             float edep)
    : SiStripHit(samples, time),
      layer_id_(layer_id),
      strip_id_(strip_id),
      track_id_(track_id),
      pdg_id_(pdg_id),
      sim_hit_id_(sim_hit_id),
      edep_(edep) {}

void SimSiStripHit::clear() {
  clearBase();
  layer_id_ = -1;
  strip_id_ = -1;
  track_id_ = -1;
  pdg_id_ = 0;
  sim_hit_id_ = -1;
  edep_ = 0.f;
}

std::ostream &operator<<(std::ostream &output, const SimSiStripHit &hit) {
  output << "[ SimSiStripHit ]: layer=" << hit.layer_id_
         << " strip=" << hit.strip_id_ << " Samples: { ";
  for (auto isample{0}; isample < (int)(hit.samples_.size() - 1); ++isample)
    output << hit.samples_[isample] << ", ";
  output << hit.samples_[hit.samples_.size() - 1] << " } "
         << "Time: " << hit.time_ << " track_id=" << hit.track_id_
         << " pdg_id=" << hit.pdg_id_ << " sim_hit_id=" << hit.sim_hit_id_
         << " edep=" << hit.edep_ << " MeV" << std::endl;

  return output;
}

}  // namespace ldmx

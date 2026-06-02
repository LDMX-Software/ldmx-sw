
#include "Tracking/Event/SimSiStripHit.h"

ClassImp(ldmx::SimSiStripHit);

namespace ldmx {

SimSiStripHit::SimSiStripHit(std::vector<short> samples, long time)
    : SiStripHit(samples, time) {}

void SimSiStripHit::clear() {
  clearBase();
  charge_ = 0;
  edep_ = 0;
}

std::ostream &operator<<(std::ostream &output, const SimSiStripHit &hit) {
  output << "[ SimSiStripHit ]: Samples: { ";
  for (auto isample{0}; isample < (hit.samples_.size() - 1); ++isample)
    output << hit.samples_[isample] << ", ";
  output << hit.samples_[hit.samples_.size() - 1] << " } "
         << "Time: " << hit.time_
         << " Charge: " << hit.charge_
         << " Edep: " << hit.edep_
         << std::endl;

  return output;
}

}  // namespace ldmx

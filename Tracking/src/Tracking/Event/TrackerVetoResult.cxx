#include "Tracking/Event/TrackerVetoResult.h"

ClassImp(ldmx::TrackerVetoResult);

namespace ldmx {

void TrackerVetoResult::Clear() {
  passes_veto_ = false;
  passes_tagger_veto_ = false;
  passes_recoil_veto_ = false;
}

std::ostream& operator<<(std::ostream& o, const TrackerVetoResult& c) {
  return o << "TrackerVetoResult { "
           << "passes_tagger_veto: " << c.passes_tagger_veto_ << ", "
           << "passes_tagger_veto: " << c.passes_tagger_veto_ << ", "
           << "passes_recoil_veto_: " << c.passes_recoil_veto_ << " }";
}
}  // namespace ldmx
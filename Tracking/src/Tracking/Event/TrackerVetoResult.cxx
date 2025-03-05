#include "Tracking/Event/TrackerVetoResult.h"

ClassImp(ldmx::TrackerVetoResult);

namespace ldmx {
TrackerVetoResult::TrackerVetoResult() {}

void TrackerVetoResult::Clear() {
  passes_veto_ = false;
  passes_tagger_veto_ = false;
  passes_recoil_veto_ = false;
}

void TrackerVetoResult::Print() const {
  std::cout << "TrackerVetoResult { "
            << "passes_tagger_veto: " << passes_tagger_veto_ << ", "
            << "passes_tagger_veto: " << passes_tagger_veto_ << ", "
            << "passes_recoil_veto_: " << passes_recoil_veto_ << " }"
            << std::endl;
}
}  // namespace ldmx
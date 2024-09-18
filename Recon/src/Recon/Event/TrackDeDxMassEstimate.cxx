#include "Recon/Event/TrackDeDxMassEstimate.h"

ClassImp(ldmx::TrackDeDxMassEstimate);

namespace ldmx {
TrackDeDxMassEstimate::TrackDeDxMassEstimate() {}

TrackDeDxMassEstimate::~TrackDeDxMassEstimate() { Clear(); }

void TrackDeDxMassEstimate::Clear() {
  mass_ = 0.;
  trackIndex_ = -1;
  trackType_ = -1;
}

void TrackDeDxMassEstimate::Print() const {
  std::cout << "TrackDeDxMassEstimate { "
            << "Mass: " << mass_ << ", "
            << "Track Index: " << trackIndex_ << ", "
            << "Track Type: " << trackType_ << " }" << std::endl;
}

}  // namespace ldmx

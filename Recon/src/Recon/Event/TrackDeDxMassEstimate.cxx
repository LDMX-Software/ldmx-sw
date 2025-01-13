#include "Recon/Event/TrackDeDxMassEstimate.h"

ClassImp(ldmx::TrackDeDxMassEstimate);

namespace ldmx {
TrackDeDxMassEstimate::TrackDeDxMassEstimate() {}

TrackDeDxMassEstimate::~TrackDeDxMassEstimate() { Clear(); }

void TrackDeDxMassEstimate::Clear() {
  mass_ = 0.;
  track_index_ = -1;
  track_type_ = -1;
}

void TrackDeDxMassEstimate::Print() const {
  std::cout << "TrackDeDxMassEstimate { "
            << "Mass: " << mass_ << ", "
            << "Track Index: " << track_index_ << ", "
            << "Track Type: " << track_type_ << " }" << std::endl;
}

}  // namespace ldmx

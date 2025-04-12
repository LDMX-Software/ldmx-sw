#include "Recon/Event/TrackDeDxMassEstimate.h"

ClassImp(ldmx::TrackDeDxMassEstimate);

namespace ldmx {
TrackDeDxMassEstimate::TrackDeDxMassEstimate() {}

void TrackDeDxMassEstimate::Clear() {
  theIh_ = -1.0;
  momentum_ = 9999.0;
  mass_ = 0.;
  track_index_ = -1;
  track_type_ = -1;
  pdg_id_ = 0;
}

void TrackDeDxMassEstimate::Print() const {
  std::cout << "TrackDeDxMassEstimate { "
            << "Momentum: " << momentum_ << ", "
            << "Ih: " << theIh_ << ", "
            << "Mass: " << mass_ << ", "
            << "Track Index: " << track_index_ << ", "
            << "Track Type: " << track_type_ << " }"
            << "PDG ID: " << pdg_id_ << " }" << std::endl;
}

}  // namespace ldmx

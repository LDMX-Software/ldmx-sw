#include "Recon/Event/TrackDeDxMassEstimate.h"

ClassImp(ldmx::TrackDeDxMassEstimate);

namespace ldmx {

void TrackDeDxMassEstimate::clear() {
  the_ih_ = -1.0;
  momentum_ = 9999.0;
  mass_ = 0.;
  track_index_ = -1;
  track_type_ = -1;
  pdg_id_ = 0;
}

std::ostream& operator<<(std::ostream& o, const TrackDeDxMassEstimate& c) {
  return o << "TrackDeDxMassEstimate { " << "Momentum: " << c.momentum_ << ", "
           << "Ih: " << c.the_ih_ << ", " << "Mass: " << c.mass_ << ", "
           << "Track Index: " << c.track_index_ << ", "
           << "Track Type: " << c.track_type_ << " }" << "PDG ID: " << c.pdg_id_
           << " }";
}

}  // namespace ldmx

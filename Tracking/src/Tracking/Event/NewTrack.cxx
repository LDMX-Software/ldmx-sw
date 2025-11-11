#include "Tracking/Event/NewTrack.h"

#include <iostream>

ClassImp(ldmx::NewTrack);

namespace ldmx {
std::ostream& operator<<(std::ostream& o, const NewTrack& c) {
  return o << "Track { " << "TrackID: " << c.track_id_
           << ", n_hits: " << c.n_hits_ << ", n_outliers: " << c.n_outliers_
           << ", ndf: " << c.ndf_ << ", chi2: " << c.chi2_
           << ", truthProb: " << c.truth_prob_ << ", pdgID: " << c.pdg_id_<< "] }";
}

}  // namespace ldmx

#include "Tracking/Event/Track.h"

#include <iostream>

ClassImp(ldmx::Track);

namespace ldmx {
std::ostream& operator<<(std::ostream& o, const Track& c) {
  return o << "Track { " << "TrackID: " << c.track_id_
           << ", n_hits: " << c.n_hits_ << ", n_outliers: " << c.n_outliers_
           << ", ndf: " << c.ndf_ << ", chi2: " << c.chi2_
           << ", truthProb: " << c.truthProb_ << ", pdgID: " << c.pdgID_
           << ", perigee_pars: [" << c.perigee_pars_[0] << ", "
           << c.perigee_pars_[1] << ", " << c.perigee_pars_[2] << ", "
           << c.perigee_pars_[3] << ", " << c.perigee_pars_[4] << "] }";
}

}  // namespace ldmx

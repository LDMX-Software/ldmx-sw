#include "Recon/Event/PFCandidate.h"

// STL
#include <iostream>

ClassImp(ldmx::PFCandidate);

namespace ldmx {
std::ostream& operator<<(std::ostream& o, const PFCandidate& c) {
  return o << "PFCandidate ( " << "id: " << c.pid_ << "), " << "Pxyz: ("
           << c.track_px_ << ", " << c.track_py_ << ", " << c.track_pz_
           << ") MeV/c, " << "Ecal energy: " << c.ecal_energy_ << " MeV, "
           << "Hcal energy: " << c.hcal_energy_ << " MeV, ";
}
}  // namespace ldmx

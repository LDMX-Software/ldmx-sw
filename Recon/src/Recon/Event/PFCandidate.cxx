#include "Recon/Event/PFCandidate.h"

// STL
#include <iostream>

ClassImp(ldmx::PFCandidate);

namespace ldmx {
std::ostream& operator<<(std::ostream& o, const PFCandidate& c) {
  return o << "PFCandidate ( " << "id: " << c.pid_ << "), " << "Pxyz: ("
           << c.trackPx_ << ", " << c.trackPy_ << ", " << c.trackPz_
           << ") MeV/c, " << "Ecal energy: " << c.ecalEnergy_ << " MeV, "
           << "Hcal energy: " << c.hcalEnergy_ << " MeV, ";
}
}  // namespace ldmx

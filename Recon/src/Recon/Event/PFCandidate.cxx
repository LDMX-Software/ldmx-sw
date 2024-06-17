#include "Recon/Event/PFCandidate.h"

// STL
#include <iostream>

ClassImp(ldmx::PFCandidate)

    namespace ldmx {
  void PFCandidate::print() const {
    std::cout << "PFCandidate ( "
              << "id: " << pid_ << "), "
              << "Pxyz: (" << trackPx_ << ", " << trackPy_ << ", " << trackPz_
              << ") MeV/c, "
              << "Ecal energy: " << ecalEnergy_ << " MeV, "
              << "Hcal energy: " << hcalEnergy_ << " MeV, " << std::endl;
  }
}  // namespace ldmx

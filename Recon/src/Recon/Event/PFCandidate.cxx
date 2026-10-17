#include "Recon/Event/PFCandidate.h"

// STL
#include <algorithm>
#include <iostream>

ClassImp(ldmx::PFCandidate);

namespace ldmx {
void PFCandidate::addHits(std::vector<unsigned int>& ids,
                          std::vector<float>& es,
                          const std::vector<unsigned int>& new_ids,
                          const std::vector<float>& new_es) {
  for (size_t k = 0; k < new_ids.size(); k++) {
    float e = k < new_es.size() ? new_es[k] : 0.f;
    auto it = std::find(ids.begin(), ids.end(), new_ids[k]);
    if (it == ids.end()) {
      ids.push_back(new_ids[k]);
      es.push_back(e);
    } else {
      es[it - ids.begin()] += e;
    }
  }
}

std::ostream& operator<<(std::ostream& o, const PFCandidate& c) {
  return o << "PFCandidate ( " << "id: " << c.pid_ << ", pdg: " << c.pdg_id_
           << "), " << "Pxyz: (" << c.track_px_ << ", " << c.track_py_ << ", "
           << c.track_pz_ << ") MeV/c, " << "Ecal energy: " << c.ecal_energy_
           << " MeV, "
           << "Hcal energy: " << c.hcal_energy_ << " MeV, ";
}
}  // namespace ldmx

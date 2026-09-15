#include "DQM/EcalSPElectronKinematics.h"

#include <cmath>

#include "DetDescr/SimSpecialID.h"
#include "SimCore/Event/SimTrackerHit.h"

namespace dqm {

void EcalSPElectronKinematics::configure(framework::config::Parameters& ps) {
  ecal_sp_coll_name_ = ps.get<std::string>("ecal_sp_coll_name");
  ecal_sp_pass_name_ = ps.get<std::string>("ecal_sp_pass_name");
}

void EcalSPElectronKinematics::produce(framework::Event& event) {
  histograms_.setWeight(event.getEventHeader().getWeight());

  if (!event.exists(ecal_sp_coll_name_, ecal_sp_pass_name_)) return;

  const auto& sp_hits{event.getCollection<ldmx::SimTrackerHit>(
      ecal_sp_coll_name_, ecal_sp_pass_name_)};

  for (const auto& hit : sp_hits) {
    ldmx::SimSpecialID hit_id(hit.getID());
    // Plane 31 is the ECal front face; require forward-going particle
    if (hit_id.plane() != 31) continue;
    const auto p = hit.getMomentum();
    if (p[2] <= 0) continue;
    // Only the primary beam electron
    if (hit.getTrackID() != 1 || hit.getPdgID() != 11) continue;

    float energy = hit.getEnergy();
    float px = static_cast<float>(p[0]);
    float py = static_cast<float>(p[1]);
    float pz = static_cast<float>(p[2]);
    const auto pos = hit.getPosition();
    float x = pos[0], y = pos[1];
    float pt = std::sqrt(px * px + py * py);

    event.add("EcalSPEnergy", energy);
    event.add("EcalSPPx", px);
    event.add("EcalSPPy", py);
    event.add("EcalSPPz", pz);
    event.add("EcalSPX", x);
    event.add("EcalSPY", y);

    histograms_.fill("energy", energy);
    histograms_.fill("pt", pt);
    histograms_.fill("px", px);
    histograms_.fill("py", py);
    histograms_.fill("x", x);
    histograms_.fill("y", y);

    // Use only the first matching hit
    break;
  }
}

}  // namespace dqm

DECLARE_ANALYZER(dqm::EcalSPElectronKinematics)

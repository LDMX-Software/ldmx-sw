#include "DQM/EcalRecHitDQM.h"

#include "DetDescr/EcalID.h"
#include "Ecal/Event/EcalHit.h"

namespace dqm {

void EcalRecHitDQM::configure(framework::config::Parameters& ps) {
  rec_hit_coll_ = ps.get<std::string>("rec_hit_coll");
  rec_hit_pass_ = ps.get<std::string>("rec_hit_pass");
  mip_si_energy_ = ps.get<double>("mip_si_energy");
}

void EcalRecHitDQM::onProcessStart() {
  getHistoDirectory();
  histograms_.create("n_hits", "Rec Hits per Event", 50, -0.5, 49.5);
  histograms_.create("amplitude", "Amplitude [MIPs]", 100, 0, 4);
  histograms_.create("time", "Hit Time [ns]", 100, -100, 100);
  histograms_.create("layer", "Layer", 34, -0.5, 33.5);
  histograms_.create("cell", "Cell", 432, -0.5, 431.5);
}

void EcalRecHitDQM::analyze(const framework::Event& event) {
  const auto& hits{
      event.getCollection<ldmx::EcalHit>(rec_hit_coll_, rec_hit_pass_)};
  histograms_.fill("n_hits", hits.size());
  for (const auto& hit : hits) {
    ldmx::EcalID id(hit.getID());
    histograms_.fill("amplitude", hit.getAmplitude() / mip_si_energy_);
    histograms_.fill("time", hit.getTime());
    histograms_.fill("layer", id.layer());
    histograms_.fill("cell", id.cell());
  }
}

}  // namespace dqm

DECLARE_ANALYZER(dqm::EcalRecHitDQM)

#include "DQM/TrigScintDigiVerifier.h"

namespace dqm {

void TrigScintDigiVerifier::configure(framework::config::Parameters& ps) {
  ts_simhit_coll_ = ps.get<std::string>("ts_simhit_coll");
  ts_simhit_pass_ = ps.get<std::string>("ts_simhit_pass");
  ts_digi_coll_ = ps.get<std::string>("ts_digi_coll");
  ts_digi_pass_ = ps.get<std::string>("ts_digi_pass");

  return;
}

void TrigScintDigiVerifier::analyze(const framework::Event& event) {
  // get truth information sorted into an ID based map
  auto ts_simhits = event.getCollection<ldmx::SimCalorimeterHit>(
      ts_simhit_coll_, ts_simhit_pass_);

  // sort sim hits by ID
  std::sort(ts_simhits.begin(), ts_simhits.end(),
            [](const ldmx::SimCalorimeterHit& lhs,
               const ldmx::SimCalorimeterHit& rhs) {
              return lhs.getID() < rhs.getID();
            });

  auto ts_digis{
      event.getCollection<ldmx::TrigScintHit>(ts_digi_coll_, ts_digi_pass_)};

  // sort digi hits by ID
  std::sort(ts_digis.begin(), ts_digis.end(),
            [](const ldmx::TrigScintHit& lhs, const ldmx::TrigScintHit& rhs) {
              return lhs.getID() < rhs.getID();
            });

  // Loop on the ts rechits
  ldmx_log(info) << "There are " << ts_digis.size()
                 << " ts digis in this event";
  for (const auto& ts_digi : ts_digis) {
    // skip anything that digi flagged as noise
    if (ts_digi.isNoise()) {
      ldmx_log(debug) << "Digi with raw ID " << ts_digi.getID()
                      << " and bar ID " << ts_digi.getBarID()
                      << " is flagged as noise, skipping";
      continue;
    }
    int raw_id = ts_digi.getID();
    ldmx_log(debug) << "Digi with raw ID " << raw_id << " and bar ID "
                    << ts_digi.getBarID() << " has energy "
                    << ts_digi.getEnergy() << " and amplitude "
                    << ts_digi.getAmplitude();

    // get information for this hit

    double total_sim_energy_dep = 0.;
    for (const auto& ts_simhit : ts_simhits) {
      if (raw_id == ts_simhit.getID()) {
        total_sim_energy_dep += ts_simhit.getEdep();
      } else if (raw_id < ts_simhit.getID()) {
        // later sim hits - all done
        break;
      }
    }

    ldmx_log(info) << " There are " << ts_simhits.size()
                   << " sim hits in this event, adding up to a total energy of "
                   << total_sim_energy_dep;

    histograms_.fill("sim_edep:rec_amplitude", total_sim_energy_dep,
                     ts_digi.getAmplitude());
    histograms_.fill("sim_edep:rec_energy", total_sim_energy_dep,
                     ts_digi.getEnergy());
  }  // end of loop on ts digis

  return;
}

}  // namespace dqm

DECLARE_ANALYZER(dqm::TrigScintDigiVerifier);

/**
 * @file RecoilMissesEcalSkimmer.cxx
 * @brief Processor used to select events where the recoil electron misses the
 *        Ecal.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "Recon/Skims/RecoilMissesEcalSkimmer.h"

namespace recon {

RecoilMissesEcalSkimmer::RecoilMissesEcalSkimmer(const std::string& name,
                                                 framework::Process& process)
    : framework::Producer(name, process) {}

RecoilMissesEcalSkimmer::~RecoilMissesEcalSkimmer() {}

void RecoilMissesEcalSkimmer::configure(
    framework::config::Parameters& parameters) {
  ecal_sim_hits_pass_name_ =
      parameters.get<std::string>("ecal_sim_hits_pass_name");

  sim_particles_pass_name_ =
      parameters.get<std::string>("sim_particles_pass_name");
}

void RecoilMissesEcalSkimmer::produce(framework::Event& event) {
  // Get the collection of simulated particles from the event
  auto particle_map{event.getMap<int, ldmx::SimParticle>(
      "SimParticles", sim_particles_pass_name_)};

  // Search for the recoil electron
  auto [recoilTrackID, recoilElectron] = analysis::getRecoil(particle_map);

  // Get the collection of simulated Ecal hits_ from the event.
  const std::vector<ldmx::SimCalorimeterHit> ecal_sim_hits =
      event.getCollection<ldmx::SimCalorimeterHit>("EcalSimHits",
                                                   ecal_sim_hits_pass_name_);

  // Loop through the Ecal hits_ and check if the recoil electron is
  // associated with any of them.  If there are any recoil electron hits_
  // in the Ecal, drop the event.
  bool has_recoil_electron_hits = false;
  for (const ldmx::SimCalorimeterHit& sim_hit : ecal_sim_hits) {
    /*std::cout << "[ RecoilMissesEcalSkimmer ]: "
              << "Number of hit contributions: "
              << simHit->getNumberOfContribs() << std::endl;*/

    for (int i_contrib = 0; i_contrib < sim_hit.getNumberOfContribs();
         ++i_contrib) {
      ldmx::SimCalorimeterHit::Contrib contrib = sim_hit.getContrib(i_contrib);

      if (contrib.track_id_ == recoilTrackID) {
        /*std::cout << "[ RecoilMissesEcalSkimmer ]: "
                  << "Ecal hit associated with recoil electron." << std::endl;
         */

        has_recoil_electron_hits = true;
      }
    }
  }

  // Tell the skimmer to keep or drop the event based on whether there
  // were recoil electron hits_ found in the Ecal.
  if (has_recoil_electron_hits) {
    setStorageHint(framework::HINT_SHOULD_DROP);
  } else {
    setStorageHint(framework::HINT_SHOULD_KEEP);
  }
}
}  // namespace recon

DECLARE_PRODUCER(recon::RecoilMissesEcalSkimmer);

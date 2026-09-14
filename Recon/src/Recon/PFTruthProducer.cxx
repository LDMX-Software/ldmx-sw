#include "Recon/PFTruthProducer.h"

#include "SimCore/Event/SimParticle.h"
#include "SimCore/Event/SimTrackerHit.h"

namespace recon {

void PFTruthProducer::configure(framework::config::Parameters& ps) {
  primary_coll_name_ = ps.get<std::string>("output_primary_coll_name");
  target_coll_name_ = ps.get<std::string>("output_target_coll_name");
  ecal_coll_name_ = ps.get<std::string>("output_ecal_coll_name");
  hcal_coll_name_ = ps.get<std::string>("output_hcal_coll_name");
  target_sp_passname_ = ps.get<std::string>("target_sp_passname");
  ecal_sp_passname_ = ps.get<std::string>("ecal_sp_passname");
  sim_particles_coll_name_ = ps.get<std::string>("sim_particles_coll_name");
  sim_particles_passname_ = ps.get<std::string>("sim_particles_passname");
  sim_particles_event_passname_ =
      ps.get<std::string>("sim_particles_event_passname");
  ecal_sp_coll_name_ = ps.get<std::string>("ecal_sp_hits_event_passname");
  ecal_sp_hits_event_passname_ =
      ps.get<std::string>("ecal_sp_hits_event_passname");
  target_sp_coll_name_ = ps.get<std::string>("target_sp_coll_name");
  target_sp_hits_event_passname_ =
      ps.get<std::string>("target_sp_hits_event_passname");
}
template <class T>
void sortHits(std::vector<T> spHits) {
  std::sort(spHits.begin(), spHits.end(),
            [](T a, T b) { return a.getEnergy() > b.getEnergy(); });
}

void PFTruthProducer::produce(framework::Event& event) {
  if (!event.exists(target_sp_coll_name_, target_sp_hits_event_passname_))
    return;
  if (!event.exists(ecal_sp_coll_name_, ecal_sp_hits_event_passname_)) return;
  if (!event.exists(sim_particles_coll_name_, sim_particles_event_passname_))
    return;
  const auto targ_sp_hits = event.getCollection<ldmx::SimTrackerHit>(
      target_sp_coll_name_, target_sp_passname_);
  const auto ecal_sp_hits = event.getCollection<ldmx::SimTrackerHit>(
      ecal_sp_coll_name_, ecal_sp_passname_);
  const auto particle_map = event.getMap<int, ldmx::SimParticle>(
      sim_particles_coll_name_, sim_particles_passname_);

  std::map<int, ldmx::SimParticle> primaries;
  std::set<int> sim_i_ds;
  std::vector<ldmx::SimTrackerHit> at_target;
  std::vector<ldmx::SimTrackerHit> at_ecal;
  std::vector<ldmx::SimTrackerHit> at_hcal;
  for (const auto& pm : particle_map) {
    const auto& p = pm.second;
    // sim particles only ever have exactly one parent
    auto parents = p.getParents();
    auto parent = parents.at(0);
    // the parent of a primary is "track 0"
    if (parent == 0) {
      primaries[pm.first] = p;
      sim_i_ds.insert(pm.first);
    }
  }
  for (const auto& sp_hit : targ_sp_hits) {
    if (sim_i_ds.count(sp_hit.getTrackID()) &&
        fabs(0.18 - sp_hit.getPosition()[2]) < 0.1 &&
        sp_hit.getMomentum()[2] > 0) {
      at_target.push_back(sp_hit);
    }
  }
  for (const auto& sp_hit : ecal_sp_hits) {
    if (sim_i_ds.count(sp_hit.getTrackID()) &&
        fabs(240 - sp_hit.getPosition()[2]) < 0.1 &&
        sp_hit.getMomentum()[2] > 0) {
      at_ecal.push_back(sp_hit);
    }
    if (sim_i_ds.count(sp_hit.getTrackID()) &&
        fabs(840 - sp_hit.getPosition()[2]) < 0.1 &&
        sp_hit.getMomentum()[2] > 0) {
      at_hcal.push_back(sp_hit);
    }
  }
  // sortHits(primaries); // use map instead
  sortHits(at_target);
  sortHits(at_ecal);
  sortHits(at_hcal);
  event.add(primary_coll_name_, primaries);
  event.add(target_coll_name_, at_target);
  event.add(ecal_coll_name_, at_ecal);
  event.add(hcal_coll_name_, at_hcal);
}
}  // namespace recon

DECLARE_PRODUCER(recon::PFTruthProducer);

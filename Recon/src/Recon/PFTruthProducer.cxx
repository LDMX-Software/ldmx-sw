#include "Recon/PFTruthProducer.h"

#include "SimCore/Event/SimParticle.h"
#include "SimCore/Event/SimTrackerHit.h"

namespace recon {

void PFTruthProducer::configure(framework::config::Parameters &ps) {
  primaryCollName_ = ps.getParameter<std::string>("outputPrimaryCollName");
  targetCollName_ = ps.getParameter<std::string>("outputTargetCollName");
  ecalCollName_ = ps.getParameter<std::string>("outputEcalCollName");
  hcalCollName_ = ps.getParameter<std::string>("outputHcalCollName");
  target_sp_passname_ = ps.getParameter<std::string>("target_sp_passname");
  ecal_sp_passname_ = ps.getParameter<std::string>("ecal_sp_passname");
  sim_particles_passname_ =
      ps.getParameter<std::string>("sim_particles_passname");
  sim_particles_event_passname_ =
      ps.getParameter<std::string>("sim_particles_event_passname");
  ecal_sp_hits_event_passname_ =
      ps.getParameter<std::string>("ecal_sp_hits_event_passname");
  target_sp_hits_event_passname_ =
      ps.getParameter<std::string>("target_sp_hits_event_passname");
}
template <class T>
void sortHits(std::vector<T> spHits) {
  std::sort(spHits.begin(), spHits.end(),
            [](T a, T b) { return a.getEnergy() > b.getEnergy(); });
}

void PFTruthProducer::produce(framework::Event &event) {
  if (!event.exists("TargetScoringPlaneHits", target_sp_hits_event_passname_))
    return;
  if (!event.exists("EcalScoringPlaneHits", ecal_sp_hits_event_passname_))
    return;
  if (!event.exists("SimParticles", sim_particles_event_passname_)) return;
  const auto targSpHits = event.getCollection<ldmx::SimTrackerHit>(
      "TargetScoringPlaneHits", target_sp_passname_);
  const auto ecalSpHits = event.getCollection<ldmx::SimTrackerHit>(
      "EcalScoringPlaneHits", ecal_sp_passname_);
  const auto particle_map = event.getMap<int, ldmx::SimParticle>(
      "SimParticles", sim_particles_passname_);

  std::map<int, ldmx::SimParticle> primaries;
  std::set<int> simIDs;
  std::vector<ldmx::SimTrackerHit> atTarget;
  std::vector<ldmx::SimTrackerHit> atEcal;
  std::vector<ldmx::SimTrackerHit> atHcal;
  for (const auto &pm : particle_map) {
    const auto &p = pm.second;
    // sim particles only ever have exactly one parent
    auto parents = p.getParents();
    auto parent = parents.at(0);
    // the parent of a primary is "track 0"
    if (parent == 0) {
      primaries[pm.first] = p;
      simIDs.insert(pm.first);
    }
  }
  for (const auto &spHit : targSpHits) {
    if (simIDs.count(spHit.getTrackID()) &&
        fabs(0.18 - spHit.getPosition()[2]) < 0.1 &&
        spHit.getMomentum()[2] > 0) {
      atTarget.push_back(spHit);
    }
  }
  for (const auto &spHit : ecalSpHits) {
    if (simIDs.count(spHit.getTrackID()) &&
        fabs(240 - spHit.getPosition()[2]) < 0.1 &&
        spHit.getMomentum()[2] > 0) {
      atEcal.push_back(spHit);
    }
    if (simIDs.count(spHit.getTrackID()) &&
        fabs(840 - spHit.getPosition()[2]) < 0.1 &&
        spHit.getMomentum()[2] > 0) {
      atHcal.push_back(spHit);
    }
  }
  // sortHits(primaries); // use map instead
  sortHits(atTarget);
  sortHits(atEcal);
  sortHits(atHcal);
  event.add(primaryCollName_, primaries);
  event.add(targetCollName_, atTarget);
  event.add(ecalCollName_, atEcal);
  event.add(hcalCollName_, atHcal);
}
}  // namespace recon

DECLARE_PRODUCER(recon::PFTruthProducer);

#include "DQM/ReSimVerifier.h"
namespace dqm {

void ReSimVerifier::configure(framework::config::Parameters& parameters) {
  sim_pass_name_ = parameters.get<std::string>("sim_pass_name");
  re_sim_pass_name_ = parameters.get<std::string>("resim_pass_name");
  stop_on_error_ = parameters.get<bool>("stop_on_error");
  collections_ = parameters.get<std::vector<std::string>>("collections");
}
bool ReSimVerifier::verifySimCalorimeterHits(
    const std::vector<ldmx::SimCalorimeterHit>& simHits,
    const std::vector<ldmx::SimCalorimeterHit>& reSimHits) {
  for (auto i{0}; i < simHits.size(); ++i) {
    auto hit{simHits[i]};
    auto rehit{reSimHits[i]};

    if (hit.getEdep() != rehit.getEdep()) {
      return false;
    }
    if (hit.getID() != rehit.getID()) {
      return false;
    }
    if (hit.getTime() != rehit.getTime()) {
      return false;
    }
    if (hit.getNumberOfContribs() != rehit.getNumberOfContribs()) {
      return false;
    }
    auto pos{hit.getPosition()};
    auto repos{rehit.getPosition()};
    if (pos[0] != repos[0] || pos[1] != repos[1] || pos[2] != repos[2]) {
      return false;
    }
  }
  return true;
}

bool ReSimVerifier::verifySimParticles(const framework::Event& event) {
  const auto& sim_particles{
      event.getMap<int, ldmx::SimParticle>("SimParticles", sim_pass_name_)};
  const auto& re_sim_particles{
      event.getMap<int, ldmx::SimParticle>("SimParticles", re_sim_pass_name_)};
  for (auto [id, simParticle] : sim_particles) {
    if (!re_sim_particles.count(id)) {
      return false;
    }
    auto re_sim_particle{re_sim_particles.at(id)};
    if (simParticle.getEnergy() != re_sim_particle.getEnergy()) {
      return false;
    }
    if (simParticle.getPdgID() != re_sim_particle.getPdgID()) {
      return false;
    }
    if (simParticle.getTime() != re_sim_particle.getTime()) {
      return false;
    }
  }
  return true;
}
void ReSimVerifier::analyze(const framework::Event& event) {
  std::stringstream ss;
  bool passing{true};
  bool skipped{false};
  auto event_number{event.getEventNumber()};
  for (auto collection : collections_) {
    const auto sim_hits = event.getCollection<ldmx::SimCalorimeterHit>(
        collection, sim_pass_name_);
    const auto re_sim_hits = event.getCollection<ldmx::SimCalorimeterHit>(
        collection, re_sim_pass_name_);
    if (re_sim_hits.size() == 0) {
      skipped = true;
      continue;
    } else {
      skipped = false;
    }

    if (!verifySimCalorimeterHits(sim_hits, re_sim_hits)) {
      passing = false;
      ss << "Event " << event_number << " has different simhits for "
         << collection << std::endl;
    }
  }
  if (skipped) {
    ldmx_log(info) << "Skipping event " << event_number
                   << "since it was not resimulated";
  }
  if (!verifySimParticles(event)) {
    passing = false;
    ss << "Event " << event_number
       << " has different SimParticles between the two passes" << std::endl;
  }
  if (!passing) {
    if (stop_on_error_) {
      EXCEPTION_RAISE("ReSimVerify", ss.str());
    } else {
      ldmx_log(info) << ss.str();
    }
  }

}  // Analyze
}  // namespace dqm
DECLARE_ANALYZER(dqm::ReSimVerifier);

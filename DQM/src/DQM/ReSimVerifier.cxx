#include "DQM/ReSimVerifier.h"
namespace dqm {

void ReSimVerifier::configure(framework::config::Parameters& parameters) {
  simPassName_ = parameters.getParameter<std::string>("sim_pass_name");
  reSimPassName_ = parameters.getParameter<std::string>("resim_pass_name");
  stop_on_error = parameters.getParameter<bool>("stop_on_error");
  collections =
      parameters.getParameter<std::vector<std::string>>("collections");
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
  const auto& simParticles{
      event.getMap<int, ldmx::SimParticle>("SimParticles", simPassName_)};
  const auto& reSimParticles{
      event.getMap<int, ldmx::SimParticle>("SimParticles", reSimPassName_)};
  for (auto [id, simParticle] : simParticles) {
    if (!reSimParticles.count(id)) {
      return false;
    }
    auto reSimParticle{reSimParticles.at(id)};
    if (simParticle.getEnergy() != reSimParticle.getEnergy()) {
      return false;
    }
    if (simParticle.getPdgID() != reSimParticle.getPdgID()) {
      return false;
    }
    if (simParticle.getTime() != reSimParticle.getTime()) {
      return false;
    }
  }
  return true;
}
void ReSimVerifier::analyze(const framework::Event& event) {
  std::stringstream ss;
  bool passing{true};
  bool skipped{false};
  auto eventNumber{event.getEventNumber()};
  for (auto collection : collections) {
    const auto SimHits =
        event.getCollection<ldmx::SimCalorimeterHit>(collection, simPassName_);
    const auto ReSimHits = event.getCollection<ldmx::SimCalorimeterHit>(
        collection, reSimPassName_);
    if (ReSimHits.size() == 0) {
      skipped = true;
      continue;
    } else {
      skipped = false;
    }

    if (!verifySimCalorimeterHits(SimHits, ReSimHits)) {
      passing = false;
      ss << "Event " << eventNumber << " has different simhits for "
         << collection << std::endl;
    }
  }
  if (skipped) {
    std::cout << "Skipping event " << eventNumber
              << "since it was not resimulated" << std::endl;
  }
  if (!verifySimParticles(event)) {
    passing = false;
    ss << "Event " << eventNumber
       << " has different SimParticles between the two passes" << std::endl;
  }
  if (!passing) {
    if (stop_on_error) {
      EXCEPTION_RAISE("ReSimVerify", ss.str());
    } else {
      std::cout << ss.str();
    }
  }

}  // Analyze
}  // namespace dqm
DECLARE_ANALYZER_NS(dqm, ReSimVerifier);

#include "Tracking/Sim/HitSmearingProcessor.h"
#include <chrono>

//---< Framework >---//
#include "Framework/Exception/Exception.h"
#include "Framework/RandomNumberSeedService.h"

namespace tracking::sim {

HitSmearingProcessor::HitSmearingProcessor(const std::string &name,
                                           framework::Process &process)
    : framework::Producer(name, process) {

  normal_ = std::make_shared<std::normal_distribution<float>>(0., 1.);
}

void HitSmearingProcessor::configure(
    framework::config::Parameters &parameters) {

  // Default configuration
  input_hit_coll_ = parameters.getParameter<std::string>("input_hit_coll");
  output_hit_coll_ = parameters.getParameter<std::string>("output_hit_coll");

  sigma_u_ = parameters.getParameter<double>("sigma_u", 0.05);
  sigma_v_ = parameters.getParameter<double>("sigma_v", 0.25);
}

void HitSmearingProcessor::onNewRun(const ldmx::RunHeader &) {

  // Get the random seed service
  auto rseed{getCondition<framework::RandomNumberSeedService>(
      framework::RandomNumberSeedService::CONDITIONS_OBJECT_NAME)};

  // Create a seed and update the generator with it
  generator_.seed(rseed.getSeed(getName()));

}

void HitSmearingProcessor::produce(framework::Event &event) {

  // Get the collection of SimTrackerHits to process from the event.
  auto sim_hits{event.getCollection<ldmx::SimTrackerHit>(input_hit_coll_)};

  // Smear the hits
  std::vector<ldmx::SimTrackerHit> smeared_hits{smearHits(sim_hits)};

  // Add the new collection to the event
  event.add(output_hit_coll_, smeared_hits);
}

std::vector<ldmx::SimTrackerHit>
HitSmearingProcessor::smearHits(const std::vector<ldmx::SimTrackerHit> &hits) {

  // Copy the existing hits into a new container that will contain the smeared
  // hits. This avoids having to copy all existing hit information such as
  // ID's.
  std::vector<ldmx::SimTrackerHit> smeared_hits(hits);

  // Loop through all the hits and smear their positions
  for (auto &hit : smeared_hits)
    smearHit(hit);

  return smeared_hits;
}

void HitSmearingProcessor::smearHit(ldmx::SimTrackerHit &hit) {

  // Get the sim hit position
  auto sim_hit_pos{hit.getPosition()};

  //This smearing is wrong. Should be done in local coordinates and not in global coordinates
  
  // LDMX Global X, along the less sensitive direction
  float smear_factor{(*normal_)(generator_)};
  sim_hit_pos[0] += smear_factor * sigma_v_;

  // LDMX Global Y, along the sensitive direction
  smear_factor = (*normal_)(generator_);
  sim_hit_pos[1] += smear_factor * sigma_u_;

  // Set the smeared hit position
  hit.setPosition(sim_hit_pos[0], sim_hit_pos[1], sim_hit_pos[2]);
  
}
} // namespace tracking::sim

DECLARE_PRODUCER_NS(tracking::sim, HitSmearingProcessor)

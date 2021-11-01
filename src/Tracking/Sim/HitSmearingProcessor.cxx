#include "Tracking/Sim/HitSmearingProcessor.h"

using namespace framework;

namespace tracking {
namespace sim {

HitSmearingProcessor::HitSmearingProcessor(const std::string &name,
                                           framework::Process &process)
    : framework::Producer(name, process) {}

HitSmearingProcessor::~HitSmearingProcessor() {}

void HitSmearingProcessor::onProcessStart() {
  normal_ = std::make_shared<std::normal_distribution<float>>(0., 1.);
}

void HitSmearingProcessor::configure(
    framework::config::Parameters &parameters) {

  // Default configuration
  input_hit_coll_ =
      parameters.getParameter<std::vector<std::string>>("input_hit_coll");
  output_hit_coll_ =
      parameters.getParameter<std::vector<std::string>>("output_hit_coll");

  tagger_sigma_u_ = parameters.getParameter<double>("tagger_sigma_u", 0.05);
  tagger_sigma_v_ = parameters.getParameter<double>("tagger_sigma_v", 0.25);

  recoil_sigma_u_ = parameters.getParameter<double>("recoil_sigma_u", 0.05);
  recoil_sigma_v_ = parameters.getParameter<double>("recoil_sigma_v", 0.25);
}

void HitSmearingProcessor::produce(framework::Event &event) {

  if (input_hit_coll_.size() != output_hit_coll_.size()) {
    std::cout << "ERROR::Size of the collections are different::"
              << input_hit_coll_.size() << "!=" << output_hit_coll_.size()
              << std::endl;
    return;
  }

  for (auto i_coll{0}; i_coll < input_hit_coll_.size(); i_coll++) {

    auto sim_hits {
      event.getCollection<ldmx::SimTrackerHit>(input_hit_coll_[i_coll])
    };

    // TODO Should convert to digi hits at this point
    std::vector<ldmx::SimTrackerHit> smeared_hits;

    for (auto &sim_hit : sim_hits) {
      smeared_hits.push_back(smearSimHit(sim_hit));
    }
    event.add(output_hit_coll_[i_coll], smeared_hits);
  }

} // produce

// This method smears the SimHit according to two independent gaussian
// distributions in u and v direction. Tagger and Recoil hits can be smeared
// with different sigma factors. The hits generated in the two detectors are
// distinguished by checking the location along the beam.

ldmx::SimTrackerHit
HitSmearingProcessor::smearSimHit(const ldmx::SimTrackerHit &hit) {

  auto sim_hit_pos{hit.getPosition()};
  ldmx::SimTrackerHit smeared_hit;

  auto sigma_u{tagger_sigma_u_};
  auto sigma_v{tagger_sigma_v_};

  // Check if the sim hit is in the tagger or in the recoil to choose the
  // smearing factor.
  if (sim_hit_pos[2] > 0) {
    sigma_u = recoil_sigma_u_;
    sigma_v = recoil_sigma_v_;
  }

  // LDMX Global X, along the less sensitive direction
  float smear_factor{(*normal_)(generator_)};
  sim_hit_pos[0] += smear_factor * sigma_v;

  // LDMX Global Y, along the sensitive direction
  smear_factor = (*normal_)(generator_);
  sim_hit_pos[1] += smear_factor * sigma_u;

  // Fill the smeared hit

  // The ID will be the same
  smeared_hit.setID(hit.getID());

  smeared_hit.setLayerID(hit.getLayerID());
  smeared_hit.setModuleID(hit.getModuleID());
  smeared_hit.setPosition(sim_hit_pos[0], sim_hit_pos[1], sim_hit_pos[2]);
  smeared_hit.setEdep(hit.getEdep());
  smeared_hit.setEnergy(hit.getEnergy());
  smeared_hit.setTime(hit.getTime());

  // Change path-length will be the same
  smeared_hit.setPathLength(hit.getPathLength());

  smeared_hit.setMomentum(hit.getMomentum()[0], hit.getMomentum()[1],
                          hit.getMomentum()[2]);
  smeared_hit.setTrackID(hit.getTrackID());
  smeared_hit.setPdgID(hit.getPdgID());
}
} // namespace sim
} // namespace tracking

DECLARE_PRODUCER_NS(tracking::sim, HitSmearingProcessor)


#include "TrigScint/TruthHitProducer.h"

namespace trigscint {

TruthHitProducer::TruthHitProducer(const std::string &name,
                                   framework::Process &process)
    : Producer(name, process) {}

void TruthHitProducer::configure(framework::config::Parameters &parameters) {
  inputCollection_ = parameters.getParameter<std::string>("input_collection");
  inputPassName_ = parameters.getParameter<std::string>("input_pass_name");
  outputCollection_ = parameters.getParameter<std::string>("output_collection");
  sim_particles_passname_ =
      parameters.getParameter<std::string>("sim_particles_passname");
  input_collection_events_passname_ =
      parameters.getParameter<std::string>("input_collection_events_passname");

  verbose_ = parameters.getParameter<bool>("verbose");

  if (verbose_) {
    ldmx_log(info) << "In TruthHitProducer: configure done!";
    ldmx_log(info) << "Got parameters:  " << "\nInput collection:     "
                   << inputCollection_
                   << "\nInput pass name:     " << inputPassName_
                   << "\nOutput collection:    " << outputCollection_
                   << "\nVerbose: " << verbose_;
  }
}

void TruthHitProducer::produce(framework::Event &event) {
  // Check if the collection exists.  If not, don't bother processing the event.
  if (!event.exists(inputCollection_, input_collection_events_passname_)) {
    ldmx_log(error) << "No input collection called " << inputCollection_
                    << " found; skipping!";
    return;
  }
  // looper over sim hits and aggregate energy depositions for each detID
  const auto sim_hits{event.getCollection<ldmx::SimCalorimeterHit>(
      inputCollection_, inputPassName_)};
  auto particle_map{event.getMap<int, ldmx::SimParticle>(
      "SimParticles", sim_particles_passname_)};

  std::vector<ldmx::SimCalorimeterHit> truth_beam_electrons;

  // TODO: Convert this to using a for_each and lambda
  for (const auto &sim_hit : sim_hits) {
    bool keep{false};
    // check if hit is from beam electron and, if so, add to output collection
    for (int i = 0; i < sim_hit.getNumberOfContribs(); i++) {
      auto contrib = sim_hit.getContrib(i);
      if (verbose_) {
        ldmx_log(debug) << "contrib " << i << " trackID: " << contrib.trackID
                        << " pdgID: " << contrib.pdgCode
                        << " edep: " << contrib.edep;
        ldmx_log(debug) << "\t particle id: "
                        << particle_map[contrib.trackID].getPdgID()
                        << " particle status: "
                        << particle_map[contrib.trackID].getGenStatus();
      }
      // if the trackID is in the map
      if (particle_map.find(contrib.trackID) != particle_map.end()) {
        // beam electron (PDGID = 11, genStatus == 1)
        if (particle_map[contrib.trackID].getPdgID() == 11 &&
            particle_map[contrib.trackID].getGenStatus() == 1) {
          keep = true;
        }
      }
      if (keep) truth_beam_electrons.push_back(sim_hit);
    }
  }
  event.add(outputCollection_, truth_beam_electrons);
}
}  // namespace trigscint

DECLARE_PRODUCER(trigscint::TruthHitProducer)

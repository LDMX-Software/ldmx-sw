
#include "TrigScint/TruthHitProducer.h"

namespace trigscint {

TruthHitProducer::TruthHitProducer(const std::string &name,
                                   framework::Process &process)
    : Producer(name, process) {}

void TruthHitProducer::configure(framework::config::Parameters &parameters) {
  input_collection_ = parameters.get<std::string>("input_collection");
  input_pass_name_ = parameters.get<std::string>("input_pass_name");
  output_collection_ = parameters.get<std::string>("output_collection");
  sim_particles_passname_ =
      parameters.get<std::string>("sim_particles_passname");
  input_collection_events_passname_ =
      parameters.get<std::string>("input_collection_events_passname");

  verbose_ = parameters.get<bool>("verbose");

  if (verbose_) {
    ldmx_log(info) << "In TruthHitProducer: configure done!";
    ldmx_log(info) << "Got parameters:  " << "\nInput collection:     "
                   << input_collection_
                   << "\nInput pass name:     " << input_pass_name_
                   << "\nOutput collection:    " << output_collection_
                   << "\nVerbose: " << verbose_;
  }
}

void TruthHitProducer::produce(framework::Event &event) {
  // Check if the collection exists.  If not, don't bother processing the event.
  if (!event.exists(input_collection_, input_collection_events_passname_)) {
    ldmx_log(error) << "No input collection called " << input_collection_
                    << " found; skipping!";
    return;
  }
  // looper over sim hits and aggregate energy depositions for each detID
  const auto sim_hits{event.getCollection<ldmx::SimCalorimeterHit>(
      input_collection_, input_pass_name_)};
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
        ldmx_log(debug) << "contrib " << i << " trackID: " << contrib.track_id_
                        << " pdgID: " << contrib.pdg_code_
                        << " edep: " << contrib.edep_;
        ldmx_log(debug) << "\t particle id: "
                        << particle_map[contrib.track_id_].getPdgID()
                        << " particle status: "
                        << particle_map[contrib.track_id_].getGenStatus();
      }
      // if the trackID is in the map
      if (particle_map.find(contrib.track_id_) != particle_map.end()) {
        // beam electron (PDGID = 11, genStatus == 1)
        if (particle_map[contrib.track_id_].getPdgID() == 11 &&
            particle_map[contrib.track_id_].getGenStatus() == 1) {
          keep = true;
        }
      }
      if (keep) truth_beam_electrons.push_back(sim_hit);
    }
  }
  event.add(output_collection_, truth_beam_electrons);
}
}  // namespace trigscint

DECLARE_PRODUCER(trigscint::TruthHitProducer)

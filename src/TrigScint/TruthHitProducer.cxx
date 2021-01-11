
#include "TrigScint/TruthHitProducer.h"

namespace trigscint {

TruthHitProducer::TruthHitProducer(const std::string &name, framework::Process&process)
    : Producer(name, process) {}

TruthHitProducer::~TruthHitProducer() {}

void TruthHitProducer::configure(framework::config::Parameters &parameters) {
  inputCollection_ = parameters.getParameter<std::string>("input_collection");
  inputPassName_ = parameters.getParameter<std::string>("input_pass_name");
  outputCollection_ = parameters.getParameter<std::string>("output_collection");
  verbose_ = parameters.getParameter<bool>("verbose");

  if (verbose_) {
    ldmx_log(info) << "In TruthHitProducer: configure done!";
    ldmx_log(info) << "Got parameters:  "
                   << "\nInput collection:     " << inputCollection_
                   << "\nInput pass name:     " << inputPassName_
                   << "\nOutput collection:    " << outputCollection_
                   << "\nVerbose: " << verbose_;
  }
}

void TruthHitProducer::produce(framework::Event &event) {
  // Check if the collection exists.  If not, don't bother processing the event.
  if (!event.exists(inputCollection_)) {
    ldmx_log(error) << "No input collection called " << inputCollection_
                    << " found; skipping!";
    return;
  }
  // looper over sim hits and aggregate energy depositions for each detID
  const auto simHits{
      event.getCollection<simcore::event::SimCalorimeterHit>(inputCollection_, inputPassName_)};
  auto particleMap{event.getMap<int, simcore::event::SimParticle>("SimParticles")};

  std::vector<simcore::event::SimCalorimeterHit> truthBeamElectrons;

  // TODO: Convert this to using a for_each and lambda
  for (const auto &simHit : simHits) {
    bool keep{false};
    // check if hit is from beam electron and, if so, add to output collection
    for (int i = 0; i < simHit.getNumberOfContribs(); i++) {
      auto contrib = simHit.getContrib(i);
      if (verbose_) {
        ldmx_log(debug) << "contrib " << i << " trackID: " << contrib.trackID
                        << " pdgID: " << contrib.pdgCode
                        << " edep: " << contrib.edep;
        ldmx_log(debug) << "\t particle id: "
                        << particleMap[contrib.trackID].getPdgID()
                        << " particle status: "
                        << particleMap[contrib.trackID].getGenStatus();
      }
      // if the trackID is in the map
      if (particleMap.find(contrib.trackID) != particleMap.end()) {
        // beam electron (PDGID = 11, genStatus == 1)
        if (particleMap[contrib.trackID].getPdgID() == 11 &&
            particleMap[contrib.trackID].getGenStatus() == 1) {
          keep = true;
        }
      }
      if (keep) truthBeamElectrons.push_back(simHit);
    }
  }
  event.add(outputCollection_, truthBeamElectrons);
}
}  // namespace trigscint

DECLARE_PRODUCER_NS(trigscint, TruthHitProducer)

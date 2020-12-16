#include "TrigScint/TrigScintRecHitProducer.h"
#include "Framework/RandomNumberSeedService.h"
#include "Framework/Exception/Exception.h"

#include <iostream>

namespace ldmx {

TrigScintRecHitProducer::TrigScintRecHitProducer(const std::string &name,
                                             Process &process)
    : Producer(name, process) {}

TrigScintRecHitProducer::~TrigScintRecHitProducer() {}

void TrigScintRecHitProducer::configure(Parameters &parameters) {

  // Configure this instance of the producer
  mevPerMip_ = parameters.getParameter<double>("mev_per_mip");
  pePerMip_ = parameters.getParameter<double>("pe_per_mip");
  inputCollection_ = parameters.getParameter<std::string>("input_collection");
  inputPassName_ = parameters.getParameter<std::string>("input_pass_name");
  outputCollection_ = parameters.getParameter<std::string>("output_collection");
  verbose_ = parameters.getParameter<bool>("verbose");

}

void TrigScintRecHitProducer::produce(Event &event) {

  // looper over sim hits and aggregate energy depositions for each detID
  const auto digis{
      event.getCollection<TrigScintQIEDigis>(inputCollection_, inputPassName_)};

  for (const auto &digi : digis) {

    TrigScintID id(digi.getID());

  }
  // Create the container to hold the digitized trigger scintillator hits.
  std::vector<TrigScintHit> trigScintHits;

  event.add(outputCollection_, trigScintHits);
}
} // namespace ldmx

DECLARE_PRODUCER_NS(ldmx, TrigScintRecHitProducer);

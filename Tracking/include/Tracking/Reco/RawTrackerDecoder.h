#pragma once

#include <string>

#include "Framework/Configure/Parameters.h"
#include "Framework/Event.h"
#include "Framework/EventProcessor.h"

namespace tracking::reco {

/**
 * Decodes raw Rogue frame data from SingleSubsystemUnpacker into a collection
 * of RawSiStripHit objects on the event bus.
 */
class RawTrackerDecoder : public framework::Producer {
 public:
  RawTrackerDecoder(const std::string& name, framework::Process& process)
      : Producer(name, process) {}

  virtual ~RawTrackerDecoder() = default;

  void configure(framework::config::Parameters& ps) override;

  void produce(framework::Event& event) override;

 private:
  /// Raw byte buffer collection name from SingleSubsystemUnpacker.
  std::string input_collection_{"TrackerRawData"};
  /// Pass name of the upstream producer.
  std::string input_pass_name_{""};
  /// Name for the output RawSiStripHit collection.
  std::string output_collection_{"RawSiStripHits"};
  /// ADC samples per hit (always 3 for APV25).
  int n_samples_{3};
};

}  // namespace tracking::reco

#ifndef TRACKING_RECO_SISTRIPWAVEFORMBUILDER_H_
#define TRACKING_RECO_SISTRIPWAVEFORMBUILDER_H_

#include <string>

#include "Framework/EventProcessor.h"
#include "Tracking/Event/SiStripWaveform.h"

namespace tracking::reco {

/**
 * Assemble per-trigger pedestal-subtracted hits into full per-channel
 * waveforms.
 */
class SiStripWaveformBuilder : public framework::Producer {
 public:
  SiStripWaveformBuilder(const std::string& name, framework::Process& process)
      : framework::Producer(name, process) {}

  void configure(framework::config::Parameters& ps) override;
  void produce(framework::Event& event) override;

 private:
  std::string input_collection_{"TrackerHits"};
  std::string input_pass_name_{""};
  std::string output_collection_{"TrackerWaveforms"};
  double high_threshold_{
      5.0};                  ///< per-sample significance for high-threshold cut
  int min_high_samples_{4};  ///< min samples exceeding high_threshold
  double low_threshold_{
      3.0};  ///< per-sample significance for consecutive-streak cut
  int min_consecutive_low_{
      5};               ///< min consecutive samples exceeding low_threshold
  int n_triggers_{10};  ///< expected APV triggers per RoR
};

}  // namespace tracking::reco

#endif  // TRACKING_RECO_SISTRIPWAVEFORMBUILDER_H_

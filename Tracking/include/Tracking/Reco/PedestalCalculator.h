#ifndef TRACKING_RECO_PEDESTALCALCULATOR_H_
#define TRACKING_RECO_PEDESTALCALCULATOR_H_

#include <array>
#include <string>
#include <unordered_map>

#include "Framework/EventProcessor.h"

namespace tracking::reco {

/**
 * Compute per-channel, per-sample pedestal mean and noise from a baseline run.
 *
 * Accumulates ADC values from every RawSiStripHit across all events, then on
 * process end writes a JSON file containing the mean and RMS noise for each
 * of the three APV25 samples.  Feed the resulting JSON to PedestalSubtractor
 * when running physics reconstruction.
 */
class PedestalCalculator : public framework::Analyzer {
 public:
  PedestalCalculator(const std::string& name, framework::Process& process)
      : framework::Analyzer(name, process) {}

  void configure(framework::config::Parameters& ps) override;
  void analyze(const framework::Event& event) override;
  void onProcessEnd() override;

 private:
  /// Write the accumulated pedestals to a JSON file (output_file_).
  void writePedestalsJson();

  // Welford's online algorithm for numerically stable mean+variance.
  struct Accumulator {
    std::array<double, 3> mean{};  // running mean
    std::array<double, 3> M2{};   // running sum of squared deviations from mean
    long n{0};                     // hit count for this channel
  };

  std::string input_collection_{"TrackerRawData"};
  std::string input_pass_name_{""};
  std::string output_file_{"pedestals.json"};
  /// Storage backend for the pedestals ("json"; future: "sqlite", ...).
  std::string output_format_{"json"};

  std::unordered_map<std::string, Accumulator> accumulators_;
  long n_events_{0};
};

}  // namespace tracking::reco

#endif  // TRACKING_RECO_PEDESTALCALCULATOR_H_

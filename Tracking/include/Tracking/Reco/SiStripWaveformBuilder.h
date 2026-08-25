#ifndef TRACKING_RECO_SISTRIPWAVEFORMBUILDER_H_
#define TRACKING_RECO_SISTRIPWAVEFORMBUILDER_H_

#include <memory>
#include <string>

#include "Framework/EventProcessor.h"
#include "Tracking/Digitization/PulseShape.h"
#include "Tracking/Event/SiStripWaveform.h"

namespace tracking::reco {

/**
 * Assemble per-trigger pedestal-subtracted hits into full per-channel
 * waveforms.
 *
 * Each RoR issues n_triggers APV triggers, each producing a 3-sample
 * (pedestal-subtracted) RawSiStripHit per channel.  This producer groups all
 * trigger hits for the same (feb, hybrid, pchannel), sorts by apv_trigger, and
 * concatenates their samples into a SiStripWaveform with n_triggers*3 samples
 * ordered as [s0_t0, s1_t0, s2_t0, s0_t1, ...].
 *
 * The per-channel noise used for the significance cuts is obtained from the
 * TrackerPedestals conditions object (a service); the physical strip number
 * (pchannel) is derived from the hit's apv_id/channel via channelmap.
 *
 * Two-stage threshold applied before writing:
 *   1. High-threshold count: at least min_high_samples samples with
 *      ADC/noise > high_threshold (default: 4 samples > 5σ).
 *   2. Consecutive low-threshold streak: at least min_consecutive_low
 *      consecutive samples with ADC/noise > low_threshold
 *      (default: 5 consecutive samples > 3σ).
 * Both conditions must be satisfied; this strongly suppresses noise while
 * retaining real APV25 signal pulses.
 */
class SiStripWaveformBuilder : public framework::Producer {
 public:
  SiStripWaveformBuilder(const std::string& name, framework::Process& process)
      : framework::Producer(name, process) {}

  void configure(framework::config::Parameters& ps) override;
  void produce(framework::Event& event) override;
  void onProcessEnd() override;

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
      5};                ///< min consecutive samples exceeding low_threshold
  int n_triggers_{10};  ///< expected APV triggers per RoR

  /// Pulse shape used for the per-waveform fit test (built lazily in produce).
  std::unique_ptr<tracking::digitization::PulseShape> pulse_shape_;

  // Fit monitoring counters (summed over the whole job, reported in
  // onProcessEnd).
  long n_fit_attempted_{0};  ///< waveforms passed to the fitter
  long n_fit_failed_{0};     ///< fits that did not converge
};

}  // namespace tracking::reco

#endif  // TRACKING_RECO_SISTRIPWAVEFORMBUILDER_H_

#pragma once

#include <memory>
#include <string>

#include "Framework/Configure/Parameters.h"
#include "Framework/Event.h"
#include "Framework/EventProcessor.h"

#include "Tracking/Digitization/PulseShape.h"
#include "Tracking/Digitization/SiStripConstants.h"
#include "Tracking/Digitization/StripPulseFitter.h"

namespace tracking::reco {

/**
 * Fits a pulse shape to each RawSiStripHit and produces FittedSiStripHits.
 *
 * This processor is intended to run on both real and simulated data; it
 * requires only the RawSiStripHit collection and knowledge of the readout
 * pulse shape.
 *
 * Input  : collection of ldmx::RawSiStripHit
 * Output : collection of ldmx::FittedSiStripHit
 *
 * Configuration parameters
 * ------------------------
 *   in_collection         Name of the RawSiStripHit input collection.
 *   in_pass               Pass name for the input collection.
 *   out_collection        Name of the FittedSiStripHit output collection.
 *   pulse_shape           "CRRC" (default) or "FourPole".
 *   tp                    Peaking time tp [ns]  (default 45).
 *   tp2                   Second time constant [ns] for FourPole (default 10).
 *   t0_offset_ns          Time of sample 0 relative to the hit frame [ns] (default 0).
 *   sampling_interval_ns  Sample spacing [ns] (default 25).
 *   pedestal_adc          Per-sample pedestal [ADC counts] (default 0).
 *   noise_sigma_adc       Per-sample noise RMS [ADC counts] used for χ² (default 5).
 *   t_scan_min_ns         Lower bound of the hit-time scan [ns] (default −50).
 *   t_scan_max_ns         Upper bound of the hit-time scan [ns] (default 150).
 *   t_scan_step_ns        Step size of the coarse scan [ns] (default 1).
 *   max_chi2_ndf          If > 0, discard fits with χ²/ndf above this value (default −1 = off).
 */
class StripFitProcessor : public framework::Producer {
 public:
  StripFitProcessor(const std::string& name, framework::Process& process);
  virtual ~StripFitProcessor() = default;

  void configure(framework::config::Parameters& parameters) override;

  void onProcessStart() override;

  void produce(framework::Event& event) override;

 private:
  // I/O
  std::string in_collection_{"RawSiStripHits"};
  std::string in_pass_{""};
  std::string out_collection_{"FittedSiStripHits"};

  // Scan range for hit-time search
  double t_scan_min_ns_{-50.0};
  double t_scan_max_ns_{150.0};
  double t_scan_step_ns_{1.0};

  // Quality cut (< 0 means disabled)
  double max_chi2_ndf_{-1.0};

  // Owned objects constructed in onProcessStart
  std::unique_ptr<tracking::digitization::PulseShape>       pulse_shape_;
  std::unique_ptr<tracking::digitization::StripPulseFitter> fitter_;
};

}  // namespace tracking::reco

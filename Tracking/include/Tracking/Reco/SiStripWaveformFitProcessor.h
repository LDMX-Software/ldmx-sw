#ifndef TRACKING_RECO_SISTRIPWAVEFORMFITPROCESSOR_H_
#define TRACKING_RECO_SISTRIPWAVEFORMFITPROCESSOR_H_

#include <map>
#include <memory>
#include <string>

#include "Framework/EventProcessor.h"
#include "Tracking/Digitization/PulseShape.h"
#include "Tracking/Reco/TrackerDaqMap.h"

namespace tracking::reco {

/**
 * Fit a pulse shape to each SiStripWaveform and produce FittedSiStripHits.
 *
 * This is the real-data counterpart of StripFitProcessor, and the bridge into
 * the geometry-aware reconstruction.  The two processors do the same thing and
 * share the same fitter; they differ only in where the sensor address and the
 * per-sample noise come from:
 *
 *   StripFitProcessor            SimSiStripHit    layer/strip from the hit,
 *                                                 noise = NOISE_SIGMA_ADC
 *   SiStripWaveformFitProcessor  SiStripWaveform  layer/strip from the DAQ map,
 *                                                 noise from TrackerPedestals
 *
 * Address mapping is applied first, so a waveform from an unmapped hybrid or an
 * unbonded channel is discarded before the (more expensive) fit runs.  Once
 * emitted, hits flow through the unchanged StripClusterProcessor -> Measurement
 * path, which knows nothing about electronics.
 *
 * Input  : collection of ldmx::SiStripWaveform (pedestal-subtracted samples)
 * Output : collection of ldmx::FittedSiStripHit
 *
 * Configuration parameters
 * ------------------------
 *   input_collection    SiStripWaveform input collection.
 *   input_pass_name     Pass name for the input collection.
 *   output_collection   FittedSiStripHit output collection.
 *   daq_map_file        Path to the DAQ map JSON (required).
 *   t_scan_min_ns       Lower bound of the hit-time scan [ns] (default -50).
 *   t_scan_max_ns       Upper bound of the hit-time scan [ns]; <= 0 means auto,
 *                       i.e. n_samples * sampling interval (default -1).
 *   t_scan_step_ns      Step size of the coarse scan [ns] (default 1).
 *   max_chi2_ndf        If > 0, discard fits with chi2/ndf above this value
 *                       (default -1 = off).
 *
 * The pedestal and the sample-0 time offset are fixed at zero: the input
 * samples are already pedestal-subtracted and their times are measured from
 * sample 0.
 */
class SiStripWaveformFitProcessor : public framework::Producer {
 public:
  SiStripWaveformFitProcessor(const std::string& name,
                              framework::Process& process)
      : framework::Producer(name, process) {}

  void configure(framework::config::Parameters& ps) override;
  void onProcessStart() override;
  void produce(framework::Event& event) override;
  void onProcessEnd() override;

 private:
  std::string input_collection_{"TrackerWaveforms"};
  std::string input_pass_name_{""};
  std::string output_collection_{"FittedSiStripHits"};
  std::string daq_map_file_{""};

  // Scan range for the hit-time search.  A non-positive t_scan_max_ns_ sizes
  // the scan to each waveform, since T may peak anywhere from before sample 0
  // to the last sample and waveforms differ in length.
  double t_scan_min_ns_{-50.0};
  double t_scan_max_ns_{-1.0};
  double t_scan_step_ns_{1.0};

  // Quality cut (<= 0 means disabled)
  double max_chi2_ndf_{-1.0};

  TrackerDaqMap daq_map_;

  /// Pulse shape shared by every per-channel fitter (built in onProcessStart).
  std::unique_ptr<tracking::digitization::PulseShape> pulse_shape_;

  // Diagnostics accumulated over the job.
  long n_waveforms_{0};      ///< waveforms seen
  long n_unmapped_{0};       ///< skipped: (feb, hybrid) absent from the DAQ map
  long n_out_of_range_{0};   ///< skipped: strip_id outside the sensor
  long n_fit_attempted_{0};  ///< waveforms passed to the fitter
  long n_unconverged_{0};    ///< skipped because the pulse fit did not converge
  long n_bad_chi2_{0};       ///< skipped by the chi2/ndf cut
  long n_hits_{0};           ///< emitted FittedSiStripHits
  /// (feb, hybrid) pairs already warned about, so each is reported only once.
  std::map<uint16_t, long> unmapped_sensors_;
};

}  // namespace tracking::reco

#endif  // TRACKING_RECO_SISTRIPWAVEFORMFITPROCESSOR_H_

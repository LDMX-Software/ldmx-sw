#include "Tracking/Reco/SiStripWaveformFitProcessor.h"

#include "Framework/Exception/Exception.h"
#include "Tracking/Digitization/SiStripConstants.h"
#include "Tracking/Digitization/StripPulseFitter.h"
#include "Tracking/Event/FittedSiStripHit.h"
#include "Tracking/Event/SiStripWaveform.h"
#include "Tracking/Reco/SiStripChannelMap.h"
#include "Tracking/Reco/TrackerPedestals.h"

namespace tracking::reco {

void SiStripWaveformFitProcessor::configure(framework::config::Parameters& ps) {
  input_collection_ =
      ps.get<std::string>("input_collection", input_collection_);
  input_pass_name_ = ps.get<std::string>("input_pass_name", input_pass_name_);
  output_collection_ =
      ps.get<std::string>("output_collection", output_collection_);
  daq_map_file_ = ps.get<std::string>("daq_map_file", daq_map_file_);

  t_scan_min_ns_ = ps.get<double>("t_scan_min_ns", t_scan_min_ns_);
  t_scan_max_ns_ = ps.get<double>("t_scan_max_ns", t_scan_max_ns_);
  t_scan_step_ns_ = ps.get<double>("t_scan_step_ns", t_scan_step_ns_);
  max_chi2_ndf_ = ps.get<double>("max_chi2_ndf", max_chi2_ndf_);
}

void SiStripWaveformFitProcessor::onProcessStart() {
  using namespace tracking::digitization;

  if (daq_map_file_.empty()) {
    EXCEPTION_RAISE("BadConfig",
                    "SiStripWaveformFitProcessor requires a daq_map_file.");
  }
  // Loads eagerly so a missing/malformed map fails here, at start-up, with the
  // path in the message -- never as a silently empty event stream.
  daq_map_ = TrackerDaqMap::fromJsonFile(daq_map_file_);

  pulse_shape_ = PulseShape::make(std::string(PULSE_SHAPE_NAME),
                                  PEAKING_TIME_NS, SECOND_TIME_CONST_NS);

  ldmx_log(info) << "SiStripWaveformFitProcessor configured:"
                 << "  daq_map='" << daq_map_file_ << "' (" << daq_map_.size()
                 << " sensors)" << "  shape=" << PULSE_SHAPE_NAME
                 << "  tp=" << PEAKING_TIME_NS << " ns" << "  T scan ["
                 << t_scan_min_ns_ << ", "
                 << (t_scan_max_ns_ > 0.0 ? std::to_string(t_scan_max_ns_)
                                          : std::string("auto"))
                 << "] ns" << "  step=" << t_scan_step_ns_ << " ns";
}

void SiStripWaveformFitProcessor::produce(framework::Event& event) {
  using namespace tracking::digitization;

  const auto& peds =
      getCondition<TrackerPedestals>(TrackerPedestals::CONDITIONS_NAME);

  const auto& waveforms = event.getCollection<ldmx::SiStripWaveform>(
      input_collection_, input_pass_name_);

  std::vector<ldmx::FittedSiStripHit> hits;
  hits.reserve(waveforms.size());

  for (const auto& wf : waveforms) {
    ++n_waveforms_;

    // -----------------------------------------------------------------------
    // Address first: an unmapped hybrid or an unbonded channel is dropped
    // before paying for the fit.
    // -----------------------------------------------------------------------
    const uint8_t feb = wf.getFebId();
    const uint8_t hybrid = wf.getHybridId();

    if (!daq_map_.has(feb, hybrid)) {
      ++n_unmapped_;
      const uint16_t k =
          static_cast<uint16_t>((static_cast<uint16_t>(feb) << 8) | hybrid);
      if (unmapped_sensors_[k]++ == 0) {
        ldmx_log(warn) << "No DAQ-map entry for feb=" << static_cast<int>(feb)
                       << " hybrid=" << static_cast<int>(hybrid)
                       << " -- dropping its waveforms (reported once)";
      }
      continue;
    }

    const auto& sensor = daq_map_.at(feb, hybrid);
    const int16_t pchannel = wf.getPchannel();
    const int strip_id = channelmap::stripId(
        pchannel, sensor.n_strips_, sensor.first_strip_, sensor.reversed_);

    // Reject channels that fall outside the bonded strip range (e.g. a read-out
    // but unbonded APV).  Counted so an unexpected layout shows up loudly.
    if (strip_id < sensor.first_strip_ ||
        strip_id >= sensor.first_strip_ + sensor.n_strips_) {
      ++n_out_of_range_;
      continue;
    }

    // -----------------------------------------------------------------------
    // Fit.  The noise enters the chi2, so the fitter is per channel; it is a
    // cheap value type over a shared pulse shape.
    // -----------------------------------------------------------------------
    uint8_t apv_id, channel;
    channelmap::apvChannelFromPchannel(pchannel, apv_id, channel);
    const float noise = peds.noise(feb, hybrid, apv_id, channel);

    const auto& samples = wf.getSamples();
    const int n_samples = static_cast<int>(samples.size());
    // Samples are pedestal-subtracted and lie on a uniform grid measured from
    // sample 0, so T may peak anywhere from before sample 0 to the last sample.
    const double t_scan_max = (t_scan_max_ns_ > 0.0)
                                  ? t_scan_max_ns_
                                  : n_samples * SAMPLING_INTERVAL_NS;

    StripPulseFitter fitter(*pulse_shape_,
                            /*t0_offset_ns=*/0.0, SAMPLING_INTERVAL_NS,
                            /*pedestal_adc=*/0.0,
                            /*noise_sigma_adc=*/noise, t_scan_min_ns_,
                            t_scan_max, t_scan_step_ns_);
    const auto fit = fitter.fit(samples);
    ++n_fit_attempted_;

    ldmx_log(trace) << "fit feb=" << static_cast<int>(feb)
                    << " hyb=" << static_cast<int>(hybrid)
                    << " pch=" << pchannel << " nsamp=" << n_samples
                    << " noise=" << noise
                    << " -> converged=" << (fit.converged ? "yes" : "no")
                    << " amp=" << fit.amplitude << " t0=" << fit.t0 << "ns"
                    << " chi2/ndf=" << fit.chi2 << "/" << fit.ndf << " ("
                    << (fit.ndf > 0 ? fit.chi2 / fit.ndf : 0.0) << ")";

    if (!fit.converged) {
      ++n_unconverged_;
      continue;
    }

    if (max_chi2_ndf_ > 0.0 && fit.ndf > 0) {
      if (fit.chi2 / fit.ndf > max_chi2_ndf_) {
        ++n_bad_chi2_;
        continue;
      }
    }

    hits.emplace_back(
        sensor.layer_id_, strip_id, static_cast<float>(fit.amplitude),
        static_cast<float>(fit.t0), static_cast<float>(fit.chi2), fit.ndf,
        /*track_id=*/-1, /*pdg_id=*/0, /*sim_hit_id=*/-1,
        /*edep=*/0.f, noise);
    ++n_hits_;
  }

  ldmx_log(debug) << "Produced " << hits.size() << " FittedSiStripHits from "
                  << waveforms.size() << " waveforms";

  event.add(output_collection_, hits);
}

void SiStripWaveformFitProcessor::onProcessEnd() {
  const long n_converged = n_fit_attempted_ - n_unconverged_;
  const double fail_pct =
      n_fit_attempted_ > 0
          ? 100.0 * static_cast<double>(n_unconverged_) / n_fit_attempted_
          : 0.0;
  ldmx_log(info) << "Fit summary: " << n_fit_attempted_ << " attempted, "
                 << n_converged << " converged, " << n_unconverged_
                 << " failed (" << fail_pct << "%)";
  ldmx_log(info) << "SiStripWaveformFitProcessor summary: " << n_waveforms_
                 << " waveforms -> " << n_hits_ << " hits (" << n_unmapped_
                 << " unmapped, " << n_out_of_range_ << " out-of-range, "
                 << n_unconverged_ << " unconverged, " << n_bad_chi2_
                 << " bad chi2/ndf)";
  for (const auto& [k, n] : unmapped_sensors_) {
    ldmx_log(warn) << "  unmapped feb=" << (k >> 8) << " hybrid=" << (k & 0xFF)
                   << ": " << n << " waveforms dropped";
  }
}

}  // namespace tracking::reco

DECLARE_PRODUCER(tracking::reco::SiStripWaveformFitProcessor)

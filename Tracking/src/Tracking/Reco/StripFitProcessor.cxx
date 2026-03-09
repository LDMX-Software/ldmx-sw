#include "Tracking/Reco/StripFitProcessor.h"

#include "Tracking/Event/FittedSiStripHit.h"
#include "Tracking/Event/RawSiStripHit.h"

using namespace framework;

namespace tracking::reco {

StripFitProcessor::StripFitProcessor(const std::string& name,
                                     framework::Process& process)
    : framework::Producer(name, process) {}

void StripFitProcessor::configure(framework::config::Parameters& parameters) {
  in_collection_  = parameters.get<std::string>("in_collection",  "RawSiStripHits");
  in_pass_        = parameters.get<std::string>("in_pass",        "");
  out_collection_ = parameters.get<std::string>("out_collection", "FittedSiStripHits");

  pulse_shape_name_     = parameters.get<std::string>("pulse_shape", "CRRC");
  tp_                   = parameters.get<double>("tp",  45.0);
  tp2_                  = parameters.get<double>("tp2", 10.0);

  t0_offset_ns_         = parameters.get<double>("t0_offset_ns",         0.0);
  sampling_interval_ns_ = parameters.get<double>("sampling_interval_ns", 25.0);

  pedestal_adc_         = parameters.get<double>("pedestal_adc",   0.0);
  noise_sigma_adc_      = parameters.get<double>("noise_sigma_adc", 5.0);

  t_scan_min_ns_  = parameters.get<double>("t_scan_min_ns",  -50.0);
  t_scan_max_ns_  = parameters.get<double>("t_scan_max_ns",  150.0);
  t_scan_step_ns_ = parameters.get<double>("t_scan_step_ns",   1.0);

  max_chi2_ndf_ = parameters.get<double>("max_chi2_ndf", -1.0);
}

void StripFitProcessor::onProcessStart() {
  pulse_shape_ = tracking::digitization::PulseShape::make(
      pulse_shape_name_, tp_, tp2_);

  fitter_ = std::make_unique<tracking::digitization::StripPulseFitter>(
      *pulse_shape_,
      t0_offset_ns_,
      sampling_interval_ns_,
      pedestal_adc_,
      noise_sigma_adc_,
      t_scan_min_ns_,
      t_scan_max_ns_,
      t_scan_step_ns_);

  ldmx_log(info) << "StripFitProcessor configured:"
                 << "  shape="    << pulse_shape_name_
                 << "  tp="       << tp_       << " ns"
                 << (pulse_shape_name_ == "FourPole"
                         ? "  tp2=" + std::to_string(tp2_) + " ns"
                         : "")
                 << "  pedestal=" << pedestal_adc_    << " ADC"
                 << "  noise_σ="  << noise_sigma_adc_ << " ADC"
                 << "  T scan ["  << t_scan_min_ns_ << ", "
                                  << t_scan_max_ns_ << "] ns"
                 << "  step="     << t_scan_step_ns_ << " ns";
}

void StripFitProcessor::produce(framework::Event& event) {
  const auto raw_hits =
      event.getCollection<ldmx::RawSiStripHit>(in_collection_, in_pass_);

  std::vector<ldmx::FittedSiStripHit> fitted_hits;
  fitted_hits.reserve(raw_hits.size());

  ldmx_log(debug) << "Fitting " << raw_hits.size() << " RawSiStripHits";

  for (const auto& raw : raw_hits) {
    const auto result = fitter_->fit(raw.getSamples());

    if (!result.converged) {
      ldmx_log(trace) << "Fit did not converge for layer=" << raw.getLayerID()
                      << " strip=" << raw.getStripID() << " — skipping";
      continue;
    }

    if (max_chi2_ndf_ > 0.0 && result.ndf > 0) {
      const double reduced_chi2 = result.chi2 / result.ndf;
      if (reduced_chi2 > max_chi2_ndf_) {
        ldmx_log(trace) << "χ²/ndf=" << reduced_chi2
                        << " exceeds cut=" << max_chi2_ndf_ << " — skipping";
        continue;
      }
    }

    fitted_hits.emplace_back(
        raw.getLayerID(),
        raw.getStripID(),
        static_cast<float>(result.amplitude),
        static_cast<float>(result.t0),
        static_cast<float>(result.chi2),
        result.ndf);

    ldmx_log(trace) << "Fitted: layer=" << raw.getLayerID()
                    << " strip=" << raw.getStripID()
                    << " amp=" << result.amplitude
                    << " t0=" << result.t0 << " ns"
                    << " chi2/ndf=" << result.chi2 << "/" << result.ndf;
  }

  ldmx_log(debug) << "Produced " << fitted_hits.size() << " FittedSiStripHits";

  event.add(out_collection_, fitted_hits);
}

}  // namespace tracking::reco

DECLARE_PRODUCER(tracking::reco::StripFitProcessor)

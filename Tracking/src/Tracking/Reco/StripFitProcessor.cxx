#include "Tracking/Reco/StripFitProcessor.h"

#include "Tracking/Event/FittedSiStripHit.h"
#include "Tracking/Event/RawSiStripHit.h"

using namespace framework;

namespace tracking::reco {

StripFitProcessor::StripFitProcessor(const std::string& name,
                                     framework::Process& process)
    : framework::Producer(name, process) {}

void StripFitProcessor::configure(framework::config::Parameters& parameters) {
  in_collection_ =
      parameters.get<std::string>("in_collection", "RawSiStripHits");
  in_pass_ = parameters.get<std::string>("in_pass", "");
  out_collection_ =
      parameters.get<std::string>("out_collection", "FittedSiStripHits");

  t_scan_min_ns_ = parameters.get<double>("t_scan_min_ns", -50.0);
  t_scan_max_ns_ = parameters.get<double>("t_scan_max_ns", 150.0);
  t_scan_step_ns_ = parameters.get<double>("t_scan_step_ns", 1.0);

  max_chi2_ndf_ = parameters.get<double>("max_chi2_ndf", -1.0);
}

void StripFitProcessor::onProcessStart() {
  using namespace tracking::digitization;

  pulse_shape_ = PulseShape::make(std::string(PULSE_SHAPE_NAME),
                                  PEAKING_TIME_NS, SECOND_TIME_CONST_NS);

  fitter_ = std::make_unique<tracking::digitization::StripPulseFitter>(
      *pulse_shape_, T0_OFFSET_NS, SAMPLING_INTERVAL_NS,
      static_cast<double>(ADC_PEDESTAL), NOISE_SIGMA_ADC, t_scan_min_ns_,
      t_scan_max_ns_, t_scan_step_ns_);

  ldmx_log(info) << "StripFitProcessor configured:" << "  shape="
                 << PULSE_SHAPE_NAME << "  tp=" << PEAKING_TIME_NS << " ns"
                 << "  pedestal=" << ADC_PEDESTAL << " ADC"
                 << "  noise_σ=" << NOISE_SIGMA_ADC << " ADC" << "  T scan ["
                 << t_scan_min_ns_ << ", " << t_scan_max_ns_ << "] ns"
                 << "  step=" << t_scan_step_ns_ << " ns";
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
        raw.getLayerID(), raw.getStripID(),
        static_cast<float>(result.amplitude), static_cast<float>(result.t0),
        static_cast<float>(result.chi2), result.ndf, raw.getTrackID(),
        raw.getPdgID(), raw.getSimHitID(), raw.getEdep());

    ldmx_log(trace) << "Fitted: layer=" << raw.getLayerID()
                    << " strip=" << raw.getStripID()
                    << " amp=" << result.amplitude << " t0=" << result.t0
                    << " ns" << " chi2/ndf=" << result.chi2 << "/"
                    << result.ndf;
  }

  ldmx_log(debug) << "Produced " << fitted_hits.size() << " FittedSiStripHits";

  event.add(out_collection_, fitted_hits);
}

}  // namespace tracking::reco

DECLARE_PRODUCER(tracking::reco::StripFitProcessor)

#include "Tracking/Reco/StripClusterProcessor.h"

#include <map>

#include "Acts/Definitions/Units.hpp"
#include "Tracking/Event/FittedSiStripHit.h"
#include "Tracking/Event/Measurement.h"

using namespace framework;

namespace tracking::reco {

StripClusterProcessor::StripClusterProcessor(const std::string& name,
                                             framework::Process& process)
    : TrackingGeometryUser(name, process) {}

// ---------------------------------------------------------------------------

void StripClusterProcessor::configure(
    framework::config::Parameters& parameters) {
  in_collection_  = parameters.get<std::string>("in_collection",  "FittedSiStripHits");
  in_pass_        = parameters.get<std::string>("in_pass",        "");
  out_collection_ = parameters.get<std::string>("out_collection", "StripMeasurements");

  readout_pitch_mm_ = parameters.get<double>("readout_pitch_mm", 0.060);
  sigma_v_mm_       = parameters.get<double>("sigma_v_mm",       20.0);

  seed_threshold_       = parameters.get<double>("seed_threshold",       4.0);
  neighbor_threshold_   = parameters.get<double>("neighbor_threshold",   3.0);
  cluster_threshold_    = parameters.get<double>("cluster_threshold",    4.0);
  noise_sigma_adc_      = parameters.get<double>("noise_sigma_adc",      5.0);
  mean_time_ns_         = parameters.get<double>("mean_time_ns",         0.0);
  time_window_ns_       = parameters.get<double>("time_window_ns",       -1.0);
  neighbor_delta_t_ns_  = parameters.get<double>("neighbor_delta_t_ns",  -1.0);
  max_chi2_ndf_         = parameters.get<double>("max_chi2_ndf",         -1.0);
}

// ---------------------------------------------------------------------------

void StripClusterProcessor::onProcessStart() {
  clusterer_ = std::make_unique<tracking::digitization::StripClusterer>(
      seed_threshold_, neighbor_threshold_, cluster_threshold_,
      noise_sigma_adc_, mean_time_ns_, time_window_ns_,
      neighbor_delta_t_ns_, max_chi2_ndf_);

  ldmx_log(info) << "StripClusterProcessor configured:"
                 << "  seed_thr="     << seed_threshold_
                 << "  nbr_thr="      << neighbor_threshold_
                 << "  cls_thr="      << cluster_threshold_
                 << "  noise="        << noise_sigma_adc_  << " ADC"
                 << "  pitch="        << readout_pitch_mm_ << " mm"
                 << "  sigma_v="      << sigma_v_mm_       << " mm";
}

// ---------------------------------------------------------------------------

void StripClusterProcessor::produce(framework::Event& event) {
  const auto fitted_hits =
      event.getCollection<ldmx::FittedSiStripHit>(in_collection_, in_pass_);

  ldmx_log(debug) << "Clustering " << fitted_hits.size() << " FittedSiStripHits";

  // -------------------------------------------------------------------------
  // Group fitted hits by layer.
  // -------------------------------------------------------------------------
  std::map<int, std::vector<ldmx::FittedSiStripHit>> hits_by_layer;
  for (const auto& h : fitted_hits) {
    hits_by_layer[h.getLayerID()].push_back(h);
  }

  // -------------------------------------------------------------------------
  // Cluster each layer and convert to Measurements.
  // -------------------------------------------------------------------------
  std::vector<ldmx::Measurement> measurements;

  for (const auto& [layer_id, layer_hits] : hits_by_layer) {

    auto hit_surface = geometry().getSurface(layer_id);
    if (!hit_surface) {
      ldmx_log(warn) << "No surface found for layer_id=" << layer_id
                     << " — skipping " << layer_hits.size() << " hits";
      continue;
    }

    const auto clusters = clusterer_->findClusters(layer_hits);
    ldmx_log(debug) << "  layer " << layer_id << ": "
                    << layer_hits.size() << " hits → "
                    << clusters.size() << " clusters";

    for (const auto& cl : clusters) {
      // -------------------------------------------------------------------
      // Local position: U from charge-weighted centroid, V unmeasured (= 0).
      // -------------------------------------------------------------------
      const double local_u = cl.centroid_strip * readout_pitch_mm_;
      const double sigma_u = cl.sigma_strip    * readout_pitch_mm_;
      constexpr double local_v = 0.0;

      // -------------------------------------------------------------------
      // Global position via Acts surface transform.
      // -------------------------------------------------------------------
      Acts::Vector3 dummy_momentum;
      const Acts::Vector3 global_pos =
          hit_surface->localToGlobal(geometryContext(),
                                     Acts::Vector2(local_u, local_v),
                                     dummy_momentum);

      // -------------------------------------------------------------------
      // Build the Measurement.
      // -------------------------------------------------------------------
      ldmx::Measurement meas;
      meas.setLayerID(layer_id);
      meas.setLocalPosition(static_cast<float>(local_u),
                            static_cast<float>(local_v));
      meas.setLocalCovariance(static_cast<float>(sigma_u * sigma_u),
                              static_cast<float>(sigma_v_mm_ * sigma_v_mm_));
      meas.setGlobalPosition(static_cast<float>(global_pos[0]),
                             static_cast<float>(global_pos[1]),
                             static_cast<float>(global_pos[2]));
      meas.setTime(static_cast<float>(cl.time_ns));

      ldmx_log(trace) << "  cluster: layer=" << layer_id
                      << " strips=" << cl.n_strips
                      << " u=" << local_u << " mm"
                      << " sigma_u=" << sigma_u << " mm"
                      << " t=" << cl.time_ns << " ns"
                      << " amp=" << cl.total_amplitude << " ADC";

      measurements.push_back(meas);
    }
  }

  ldmx_log(debug) << "Produced " << measurements.size() << " Measurements";
  event.add(out_collection_, measurements);
}

}  // namespace tracking::reco

DECLARE_PRODUCER(tracking::reco::StripClusterProcessor)

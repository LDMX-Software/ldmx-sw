#include "Tracking/Reco/StripClusterProcessor.h"

#include <cmath>
#include <map>
#include <unordered_set>

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

  seed_threshold_       = parameters.get<double>("seed_threshold",       4.0);
  neighbor_threshold_   = parameters.get<double>("neighbor_threshold",   3.0);
  cluster_threshold_    = parameters.get<double>("cluster_threshold",    4.0);
  mean_time_ns_         = parameters.get<double>("mean_time_ns",         0.0);
  time_window_ns_       = parameters.get<double>("time_window_ns",       -1.0);
  neighbor_delta_t_ns_  = parameters.get<double>("neighbor_delta_t_ns",  -1.0);
  max_chi2_ndf_         = parameters.get<double>("max_chi2_ndf",         -1.0);
}

// ---------------------------------------------------------------------------

void StripClusterProcessor::onProcessStart() {
  using namespace tracking::digitization;

  clusterer_ = std::make_unique<tracking::digitization::StripClusterer>(
      seed_threshold_, neighbor_threshold_, cluster_threshold_,
      NOISE_SIGMA_ADC, mean_time_ns_, time_window_ns_,
      neighbor_delta_t_ns_, max_chi2_ndf_);

  ldmx_log(info) << "StripClusterProcessor configured:"
                 << "  seed_thr="     << seed_threshold_
                 << "  nbr_thr="      << neighbor_threshold_
                 << "  cls_thr="      << cluster_threshold_
                 << "  noise="        << NOISE_SIGMA_ADC  << " ADC"
                 << "  pitch="        << READOUT_PITCH_MM << " mm"
                 << "  sigma_v="      << SIGMA_V_MM       << " mm";
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

    // Build a strip-index → FittedSiStripHit map for truth lookup.
    std::map<int, const ldmx::FittedSiStripHit*> strip_hit_map;
    for (const auto& h : layer_hits) {
      strip_hit_map[h.getStripID()] = &h;
    }

    const auto clusters = clusterer_->findClusters(layer_hits);
    ldmx_log(debug) << "  layer " << layer_id << ": "
                    << layer_hits.size() << " hits → "
                    << clusters.size() << " clusters";

    for (const auto& cl : clusters) {
      // -------------------------------------------------------------------
      // Local position: U from charge-weighted centroid, V unmeasured (= 0).
      // Strip r covers sense strips { ratio*(r-N_int), ..., +ratio-1 },
      // so its physical centre is at:
      //   U = (r - N_int + (ratio-1)/(2*ratio)) * readout_pitch
      // with N_int = integer(N/2), ratio = round(readout_pitch / sense_pitch).
      // For N=767, ratio=2: offset = 383 - 0.25 = 382.75.
      // -------------------------------------------------------------------
      using namespace tracking::digitization;
      const int    n_int_  = N_READOUT_STRIPS / 2;  // integer division
      const int    ratio_  = static_cast<int>(
          std::round(READOUT_PITCH_MM / SENSE_PITCH_MM));
      const double offset_ = n_int_ - 0.5 * (ratio_ - 1.0) / ratio_;
      const double local_u = (cl.centroid_strip - offset_) * READOUT_PITCH_MM;
      const double sigma_u = cl.sigma_strip    * READOUT_PITCH_MM;
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
                              static_cast<float>(tracking::digitization::SIGMA_V_MM
                                                 * tracking::digitization::SIGMA_V_MM));
      meas.setGlobalPosition(static_cast<float>(global_pos[0]),
                             static_cast<float>(global_pos[1]),
                             static_cast<float>(global_pos[2]));
      meas.setTime(static_cast<float>(cl.time_ns));
      meas.setNStrips(cl.n_strips);
      meas.setClusterAmplitude(static_cast<float>(cl.total_amplitude));

      // -------------------------------------------------------------------
      // Reconstructed energy: convert total cluster amplitude to edep using
      // the fixed detector constants from SiStripConstants.h.
      //   edep = total_amplitude [ADC] × ADC_ELECTRONS_PER_COUNT [e/ADC]
      //                                × ENERGY_PER_EHP_MEV [MeV/e]
      // -------------------------------------------------------------------
      const float reco_edep = static_cast<float>(
          cl.total_amplitude * ADC_ELECTRONS_PER_COUNT * ENERGY_PER_EHP_MEV);
      meas.setEdep(reco_edep);

      // -------------------------------------------------------------------
      // Truth matching: collect unique track IDs from constituent strips.
      // -------------------------------------------------------------------
      std::unordered_set<int> seen_track_ids;
      for (const int strip : cl.strip_ids) {
        auto it = strip_hit_map.find(strip);
        if (it != strip_hit_map.end()) {
          const int tid = it->second->getTrackID();
          if (tid >= 0 && seen_track_ids.insert(tid).second) {
            meas.addTrackId(tid);
          }
        }
      }

      ldmx_log(trace) << "  cluster: layer=" << layer_id
                      << " strips=" << cl.n_strips
                      << " u=" << local_u << " mm"
                      << " sigma_u=" << sigma_u << " mm"
                      << " t=" << cl.time_ns << " ns"
                      << " amp=" << cl.total_amplitude << " ADC"
                      << " n_track_ids=" << seen_track_ids.size();

      measurements.push_back(meas);
    }
  }

  ldmx_log(debug) << "Produced " << measurements.size() << " Measurements";
  event.add(out_collection_, measurements);
}

}  // namespace tracking::reco

DECLARE_PRODUCER(tracking::reco::StripClusterProcessor)

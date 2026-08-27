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
  in_collection_ =
      parameters.get<std::string>("in_collection", "FittedSiStripHits");
  in_pass_ = parameters.get<std::string>("in_pass", "");
  out_collection_ =
      parameters.get<std::string>("out_collection", "StripMeasurements");

  seed_threshold_ = parameters.get<double>("seed_threshold", 4.0);
  neighbor_threshold_ = parameters.get<double>("neighbor_threshold", 3.0);
  cluster_threshold_ = parameters.get<double>("cluster_threshold", 4.0);
  mean_time_ns_ = parameters.get<double>("mean_time_ns", 0.0);
  time_window_ns_ = parameters.get<double>("time_window_ns", -1.0);
  neighbor_delta_t_ns_ = parameters.get<double>("neighbor_delta_t_ns", -1.0);
  max_chi2_ndf_ = parameters.get<double>("max_chi2_ndf", -1.0);
  daq_map_file_ = parameters.get<std::string>("daq_map_file", "");
}

// ---------------------------------------------------------------------------

void StripClusterProcessor::onProcessStart() {
  using namespace tracking::digitization;

  clusterer_ = std::make_unique<tracking::digitization::StripClusterer>(
      seed_threshold_, neighbor_threshold_, cluster_threshold_, NOISE_SIGMA_ADC,
      mean_time_ns_, time_window_ns_, neighbor_delta_t_ns_, max_chi2_ndf_);

  // Optional DAQ map: build a layer_id -> n_strips lookup so the local-U centre
  // offset can use the real per-sensor strip count for real data.  Left empty
  // for MC, in which case the fixed N_READOUT_STRIPS constant is used below.
  layer_n_strips_.clear();
  if (!daq_map_file_.empty()) {
    const auto map = TrackerDaqMap::fromJsonFile(daq_map_file_);
    for (const auto& [key, sensor] : map.sensors()) {
      layer_n_strips_[sensor.layer_id] = sensor.n_strips;
    }
    ldmx_log(info) << "StripClusterProcessor loaded DAQ map from '"
                   << daq_map_file_ << "' (" << layer_n_strips_.size()
                   << " layers) for centre-strip offsets";
  }

  ldmx_log(info) << "StripClusterProcessor configured:" << "  seed_thr="
                 << seed_threshold_ << "  nbr_thr=" << neighbor_threshold_
                 << "  cls_thr=" << cluster_threshold_
                 << "  noise=" << NOISE_SIGMA_ADC << " ADC"
                 << "  pitch=" << READOUT_PITCH_MM << " mm"
                 << "  sigma_v=" << SIGMA_V_MM << " mm";
}

// ---------------------------------------------------------------------------

void StripClusterProcessor::produce(framework::Event& event) {
  const auto& fitted_hits =
      event.getCollection<ldmx::FittedSiStripHit>(in_collection_, in_pass_);

  ldmx_log(debug) << "Clustering " << fitted_hits.size()
                  << " FittedSiStripHits";

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
    ldmx_log(debug) << "  layer " << layer_id << ": " << layer_hits.size()
                    << " hits → " << clusters.size() << " clusters";

    for (const auto& cl : clusters) {
      // -------------------------------------------------------------------
      // Local position: U from charge-weighted centroid, V unmeasured (= 0).
      // With AC-coupled transfer efficiencies, each readout strip r is anchored
      // at the position of its paired sense strip (position_in_group == 0),
      // which is at U = (r - N_int) * readout_pitch where N_int = N/2
      // (integer). For N=767: offset = 383, so readout 383 → U=0, 384 → U=60
      // µm, etc.
      // -------------------------------------------------------------------
      using namespace tracking::digitization;
      // Centre-strip offset: N/2 (integer division).  For MC this is the fixed
      // N_READOUT_STRIPS constant; for real data, if a DAQ map was supplied,
      // use that sensor's real strip count so the local origin sits at its
      // centre.
      int n_strips = N_READOUT_STRIPS;
      auto it_ns = layer_n_strips_.find(layer_id);
      if (it_ns != layer_n_strips_.end()) n_strips = it_ns->second;
      const int n_int = n_strips / 2;
      const double offset = static_cast<double>(n_int);
      const double local_u = (cl.centroid_strip - offset) * READOUT_PITCH_MM;

      // Cluster-size-dependent position uncertainty using sense pitch (30 µm).
      // Divisors follow the HPS convention: 1/√12 for single-strip (binary
      // resolution), 1/5 for 2-strip (best charge-sharing), then degrading.
      double sigma_u;
      switch (cl.n_strips) {
        case 1:
          sigma_u = SENSE_PITCH_MM / std::sqrt(12.0);
          break;  // 8.7 µm
        case 2:
          sigma_u = SENSE_PITCH_MM / 5.0;
          break;  // 6.0 µm
        case 3:
          sigma_u = SENSE_PITCH_MM / 3.0;
          break;  // 10.0 µm
        case 4:
          sigma_u = SENSE_PITCH_MM / 2.0;
          break;  // 15.0 µm
        default:
          sigma_u = SENSE_PITCH_MM;
          break;  // 30.0 µm
      }
      constexpr double local_v = 0.0;

      // -------------------------------------------------------------------
      // Global position via Acts surface transform.
      // -------------------------------------------------------------------
      Acts::Vector3 dummy_momentum;
      const Acts::Vector3 global_pos = hit_surface->localToGlobal(
          geometryContext(), Acts::Vector2(local_u, local_v), dummy_momentum);

      // -------------------------------------------------------------------
      // Build the Measurement.
      // -------------------------------------------------------------------
      ldmx::Measurement meas;
      meas.setLayerID(layer_id);
      meas.setLocalPosition(static_cast<float>(local_u),
                            static_cast<float>(local_v));
      meas.setLocalCovariance(
          static_cast<float>(sigma_u * sigma_u),
          static_cast<float>(tracking::digitization::SIGMA_V_MM *
                             tracking::digitization::SIGMA_V_MM));
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
                      << " strips=" << cl.n_strips << " u=" << local_u << " mm"
                      << " sigma_u=" << sigma_u << " mm" << " t=" << cl.time_ns
                      << " ns" << " amp=" << cl.total_amplitude << " ADC"
                      << " n_track_ids=" << seen_track_ids.size();

      measurements.push_back(meas);
    }
  }

  ldmx_log(debug) << "Produced " << measurements.size() << " Measurements";
  event.add(out_collection_, measurements);
}

}  // namespace tracking::reco

DECLARE_PRODUCER(tracking::reco::StripClusterProcessor)

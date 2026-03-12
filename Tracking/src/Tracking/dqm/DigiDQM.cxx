#include "Tracking/dqm/DigiDQM.h"

#include <cmath>
#include <map>
#include <string>

#include "SimCore/Event/SimTrackerHit.h"
#include "Tracking/Event/FittedSiStripHit.h"
#include "Tracking/Event/Measurement.h"
#include "Tracking/Sim/TrackingUtils.h"

// Helper: decode the raw sensor index (from getSensorID / FittedSiStripHit
// layer_id) into the compact internal layer index 0..N-1 used by Measurement.
static inline int sensorIndexToLayer(int raw_id) {
  return ((raw_id / 100) % 10 - 1) * 2 + raw_id % 2;
}

namespace tracking::dqm {

void DigiDQM::configure(framework::config::Parameters& parameters) {
  sim_coll_name_     = parameters.get<std::string>("sim_coll_name",     "TaggerSimHits");
  sim_pass_name_     = parameters.get<std::string>("sim_pass_name",     "");
  digi_coll_name_    = parameters.get<std::string>("digi_coll_name",    "DigiTaggerSimHits");
  digi_pass_name_    = parameters.get<std::string>("digi_pass_name",    "");
  fitted_coll_name_  = parameters.get<std::string>("fitted_coll_name",  "TaggerFittedSiStripHits");
  fitted_pass_name_  = parameters.get<std::string>("fitted_pass_name",  "");
  cluster_coll_name_ = parameters.get<std::string>("cluster_coll_name", "TaggerClusterMeasurements");
  cluster_pass_name_ = parameters.get<std::string>("cluster_pass_name", "");
  n_sensors_         = parameters.get<int>("n_sensors", 14);
}

void DigiDQM::analyze(const framework::Event& event) {

  // -------------------------------------------------------------------------
  // Sim hits — one entry per hit
  // -------------------------------------------------------------------------
  if (event.exists(sim_coll_name_, sim_pass_name_)) {
    const auto sim_hits =
        event.getCollection<ldmx::SimTrackerHit>(sim_coll_name_, sim_pass_name_);

    // Count per sensor for the multiplicity histograms
    std::map<int, int> sim_count;  // layer → count

    for (const auto& hit : sim_hits) {
      // getSensorID maps the G4 layer/module IDs to the same compact index
      // that DigitizationProcessor stores in Measurement::layer_id_.
      int layer = sensorIndexToLayer(tracking::sim::utils::getSensorID(hit));
      if (layer < 0 || layer >= n_sensors_) continue;

      const std::string s = std::to_string(layer);
      histograms_.fill("sim_edep_s" + s, hit.getEdep());
      sim_count[layer]++;
    }

    for (int i = 0; i < n_sensors_; ++i) {
      histograms_.fill("sim_n_s" + std::to_string(i),
                       static_cast<double>(sim_count[i]));
    }
  }

  // -------------------------------------------------------------------------
  // Digi measurements (from DigitizationProcessor)
  // Build a map layer -> list of (track_id, digi_u, truth_u) for residuals.
  // -------------------------------------------------------------------------
  struct DigiEntry { int tid; float digi_u; float truth_u; };
  std::map<int, std::vector<DigiEntry>> digi_by_layer;

  if (event.exists(digi_coll_name_, digi_pass_name_)) {
    const auto digi_meas =
        event.getCollection<ldmx::Measurement>(digi_coll_name_, digi_pass_name_);

    std::map<int, int> digi_count;

    for (const auto& meas : digi_meas) {
      int layer = meas.getLayer();
      if (layer < 0 || layer >= n_sensors_) continue;

      const std::string s = std::to_string(layer);
      const float u = meas.getLocalPosition()[0];
      histograms_.fill("digi_u_s" + s, u);
      digi_count[layer]++;

      // Record for residual calculation; use first track_id if available
      int tid = meas.getTrackIds().empty() ? -1
                                           : static_cast<int>(meas.getTrackIds()[0]);
      digi_by_layer[layer].push_back({tid, u, meas.getTruthU()});
    }

    for (int i = 0; i < n_sensors_; ++i) {
      histograms_.fill("digi_n_s" + std::to_string(i),
                       static_cast<double>(digi_count[i]));
    }
  }

  // -------------------------------------------------------------------------
  // Fitted strips (from StripFitProcessor)
  // -------------------------------------------------------------------------
  if (event.exists(fitted_coll_name_, fitted_pass_name_)) {
    const auto fitted_hits =
        event.getCollection<ldmx::FittedSiStripHit>(fitted_coll_name_, fitted_pass_name_);

    std::map<int, int> strip_count;

    for (const auto& hit : fitted_hits) {
      // FittedSiStripHit::getLayerID() stores the same raw sensor index
      // as Measurement::layer_id_ (set by getSensorID in DigitizationProcessor).
      int layer = sensorIndexToLayer(hit.getLayerID());
      if (layer < 0 || layer >= n_sensors_) continue;

      const std::string s = std::to_string(layer);
      histograms_.fill("strip_amp_s"     + s, hit.getAmplitude());
      histograms_.fill("strip_t0_s"      + s, hit.getT0());
      if (hit.getNDF() > 0)
        histograms_.fill("strip_chi2ndf_s" + s, hit.getChi2() / hit.getNDF());
      strip_count[layer]++;
    }

    for (int i = 0; i < n_sensors_; ++i) {
      histograms_.fill("strip_n_s" + std::to_string(i),
                       static_cast<double>(strip_count[i]));
    }
  }

  // -------------------------------------------------------------------------
  // Cluster measurements (from StripClusterProcessor)
  // -------------------------------------------------------------------------
  if (event.exists(cluster_coll_name_, cluster_pass_name_)) {
    const auto cluster_meas =
        event.getCollection<ldmx::Measurement>(cluster_coll_name_, cluster_pass_name_);

    std::map<int, int> cluster_count;

    for (const auto& meas : cluster_meas) {
      int layer = meas.getLayer();
      if (layer < 0 || layer >= n_sensors_) continue;

      const std::string s = std::to_string(layer);
      const float u = meas.getLocalPosition()[0];
      histograms_.fill("cluster_u_s"       + s, u);
      histograms_.fill("cluster_amp_s"     + s, meas.getClusterAmplitude());
      histograms_.fill("cluster_time_s"    + s, meas.getTime());
      if (meas.getNStrips() > 0)
        histograms_.fill("cluster_nstrips_s" + s, meas.getNStrips());
      cluster_count[layer]++;

      // Residuals: find a matching digi measurement on the same layer with the
      // same dominant track_id
      int tid = meas.getTrackIds().empty() ? -1
                                           : static_cast<int>(meas.getTrackIds()[0]);
      if (tid >= 0) {
        auto it = digi_by_layer.find(layer);
        if (it != digi_by_layer.end()) {
          for (const auto& entry : it->second) {
            if (entry.tid == tid) {
              histograms_.fill("du_s" + s, entry.digi_u - u);
              // truth_u - cluster_u (only when truth_u was set)
              if (!std::isnan(entry.truth_u)) {
                histograms_.fill("sim_cluster_du_s" + s, entry.truth_u - u);
              }
              break;
            }
          }
        }
      }
    }

    for (int i = 0; i < n_sensors_; ++i) {
      histograms_.fill("cluster_n_s" + std::to_string(i),
                       static_cast<double>(cluster_count[i]));
    }
  }
}

}  // namespace tracking::dqm

DECLARE_ANALYZER(tracking::dqm::DigiDQM)

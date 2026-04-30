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
  digi_coll_name_    = parameters.get<std::string>("digi_coll_name",    "");
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
  // Build truth_u lookup from digi Measurements (optional).
  // layer → track_id → truth_u
  // -------------------------------------------------------------------------
  std::map<int, std::map<int, float>> truth_u_by_layer;
  if (!digi_coll_name_.empty() && event.exists(digi_coll_name_, digi_pass_name_)) {
    const auto digi_meas =
        event.getCollection<ldmx::Measurement>(digi_coll_name_, digi_pass_name_);
    for (const auto& m : digi_meas) {
      int layer = m.getLayer();
      if (layer < 0 || layer >= n_sensors_) continue;
      if (std::isnan(m.getTruthU())) continue;
      int tid = m.getTrackIds().empty() ? -1 : static_cast<int>(m.getTrackIds()[0]);
      truth_u_by_layer[layer][tid] = m.getTruthU();
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

      // sim_cluster_du: truth U (from digi Measurement) minus cluster U
      int tid = meas.getTrackIds().empty() ? -1 : static_cast<int>(meas.getTrackIds()[0]);
      auto layer_it = truth_u_by_layer.find(layer);
      if (layer_it != truth_u_by_layer.end()) {
        auto tid_it = layer_it->second.find(tid);
        if (tid_it != layer_it->second.end())
          histograms_.fill("sim_cluster_du_s" + s, tid_it->second - u);
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

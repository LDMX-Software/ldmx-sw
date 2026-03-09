#pragma once

#include <memory>
#include <string>

#include "Framework/Configure/Parameters.h"
#include "Framework/Event.h"

#include "Tracking/Digitization/StripClusterer.h"
#include "Tracking/Reco/TrackingGeometryUser.h"

namespace tracking::reco {

/**
 * Clusters fitted silicon-strip hits and produces Measurement objects.
 *
 * Groups FittedSiStripHit objects by sensor layer, runs the nearest-neighbour
 * clustering algorithm on each layer, then converts each accepted cluster into
 * an ldmx::Measurement using the Acts tracking geometry for the local → global
 * position transform.
 *
 * This processor runs on both real data (after pulse fitting) and simulation.
 *
 * Input  : collection of ldmx::FittedSiStripHit
 * Output : collection of ldmx::Measurement
 *
 * Configuration parameters
 * ------------------------
 *   in_collection         FittedSiStripHit input collection (default "FittedSiStripHits").
 *   in_pass               Pass name for the input collection (default "").
 *   out_collection        Measurement output collection (default "StripMeasurements").
 *   readout_pitch_mm      Readout strip pitch [mm] — converts strip index to U [mm] (default 0.060).
 *   sigma_v_mm            Position uncertainty in V [mm].  V is unmeasured by
 *                         strips, so set to strip half-length / √3 or similar
 *                         (default 20.0).
 *   seed_threshold        Minimum amplitude/noise to seed a cluster (default 4.0).
 *   neighbor_threshold    Minimum amplitude/noise for a strip to join a cluster (default 3.0).
 *   cluster_threshold     Minimum Σamp / √Σnoise² for cluster acceptance (default 4.0).
 *   noise_sigma_adc       Per-strip noise RMS [ADC counts] (default 5.0).
 *   mean_time_ns          Expected hit time for seed timing cut [ns] (default 0.0).
 *   time_window_ns        Half-width of seed timing window [ns]; ≤ 0 disables (default -1).
 *   neighbor_delta_t_ns   Max |t0_nb − cluster_t| [ns] for a neighbour to join;
 *                         ≤ 0 disables (default -1).
 *   max_chi2_ndf          Max chi2/ndf for a hit to be clustered; ≤ 0 disables (default -1).
 */
class StripClusterProcessor : public TrackingGeometryUser {
 public:
  StripClusterProcessor(const std::string& name, framework::Process& process);
  virtual ~StripClusterProcessor() = default;

  void configure(framework::config::Parameters& parameters) override;
  void onProcessStart() override;
  void produce(framework::Event& event) override;

 private:
  // I/O
  std::string in_collection_{"FittedSiStripHits"};
  std::string in_pass_{""};
  std::string out_collection_{"StripMeasurements"};

  // Geometry
  double readout_pitch_mm_{0.060};
  double sigma_v_mm_{20.0};

  // Clustering parameters (forwarded to StripClusterer)
  double seed_threshold_{4.0};
  double neighbor_threshold_{3.0};
  double cluster_threshold_{4.0};
  double noise_sigma_adc_{5.0};
  double mean_time_ns_{0.0};
  double time_window_ns_{-1.0};
  double neighbor_delta_t_ns_{-1.0};
  double max_chi2_ndf_{-1.0};

  std::unique_ptr<tracking::digitization::StripClusterer> clusterer_;
};

}  // namespace tracking::reco

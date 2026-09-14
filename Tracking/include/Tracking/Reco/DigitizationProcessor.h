#pragma once

#include "Framework/RandomNumberSeedService.h"
#include "Tracking/Reco/TrackingGeometryUser.h"

//--- ACTS ---//
#include "Acts/Definitions/Units.hpp"
#include "Acts/Surfaces/RectangleBounds.hpp"
#include "Acts/Surfaces/Surface.hpp"

//--- LDMX ---//
#include "Tracking/Digitization/PulseShape.h"
#include "Tracking/Digitization/SiStripConstants.h"
#include "Tracking/Digitization/SiStripDigitizer.h"
#include "Tracking/Event/Measurement.h"
#include "Tracking/Event/SimSiStripHit.h"
#include "Tracking/Sim/TrackingUtils.h"
#include "Tracking/geo/DetectorElement.h"

//--- C++ ---//
#include <chrono>
#include <memory>
#include <random>
#include <unordered_map>
#include <utility>

namespace ldmx {
class Measurement;
}

namespace tracking::reco {

/**
 * Digitization processor for the silicon strip tracker.
 *
 * Two modes are available, selected by the `use_charge_digitization` parameter:
 *
 *  Mode 0 (default, use_charge_digitization = false):
 *    Simple Gaussian smearing of the local coordinates.  Fast and sufficient
 *    for most studies.  Resolution set by sigma_u / sigma_v.
 *
 *  Mode 1 (use_charge_digitization = true):
 *    Realistic charge digitization.  The track segment through the sensor is
 *    divided into sub-segments; each creates electron-hole pairs (N = Edep /
 *    E_pair).  Thermal diffusion during drift spreads the charge as a Gaussian
 *    across strips.  Electronic noise is added and a threshold is applied;
 *    the surviving strip cluster is clustered by a charge-weighted centroid to
 *    produce the local U measurement.
 */
class DigitizationProcessor : public TrackingGeometryUser {
 public:
  DigitizationProcessor(const std::string& name, framework::Process& process);
  virtual ~DigitizationProcessor() = default;

  void onProcessStart() override;

  void configure(framework::config::Parameters& parameters) override;

  /**
   * Before the run starts (but after the conditions are configured)
   * set up the random seeds for this run.
   *
   * @param[in] header RunHeader for this run, unused
   */
  void onNewRun(const ldmx::RunHeader& header) override;

  void produce(framework::Event& event) override;

  /**
   * Digitize a collection of SimTrackerHits into Measurements.
   *
   * Operates in either smearing mode or full charge-digitization mode
   * depending on the `use_charge_digitization` configuration flag.
   *
   * @param sim_hits  The collection of SimTrackerHits to digitize.
   * @param raw_hits  If non-null and charge digitization is active, filled
   *                  with one SimSiStripHit per above-threshold readout strip.
   */
  std::vector<ldmx::Measurement> digitizeHits(
      const std::vector<ldmx::SimTrackerHit>& sim_hits,
      std::vector<ldmx::SimSiStripHit>* raw_hits = nullptr);

  // TODO avoid copies and use references
  bool mergeSimHits(const std::vector<ldmx::SimTrackerHit>& sim_hits,
                    std::vector<ldmx::SimTrackerHit>& merged_hits);
  bool mergeHits(const std::vector<ldmx::SimTrackerHit>& sihits,
                 std::vector<ldmx::SimTrackerHit>& mergedHits);

 private:
  /// Input hit collection to digitize.
  std::string hit_collection_;
  /// Output measurement collection name.
  std::string out_collection_;

  /// Minimum energy deposition cut [MeV].
  double min_e_dep_;
  /// Select a particular track ID (-1 = accept all).
  int track_id_;
  /// Merge sim hits on the same sensor before digitizing.
  bool merge_hits_{false};

  // -------------------------------------------------------------------------
  // Mode 0: Gaussian smearing
  // -------------------------------------------------------------------------
  /// Flag to enable/disable smearing in Mode 0.
  bool do_smearing_{true};
  /// u-direction smearing sigma [mm].
  double sigma_u_{0};
  /// v-direction smearing sigma [mm].
  double sigma_v_{0};

  // -------------------------------------------------------------------------
  // Mode 1: Realistic charge digitization
  // -------------------------------------------------------------------------
  /// If true, use the full SiStripDigitizer instead of simple smearing.
  bool use_charge_digitization_{false};
  /// Parameters forwarded to SiStripDigitizer.
  tracking::digitization::SiStripDigitizer::SensorParams sensor_params_;
  /// The charge digitizer (constructed in onProcessStart).
  std::unique_ptr<tracking::digitization::SiStripDigitizer> strip_digitizer_;

  // ADC conversion and pulse shaping (Mode 1 only)
  /// Output raw hit collection name (empty = don't save raw hits).
  std::string out_raw_collection_{""};
  /// The constructed pulse shape (created in onProcessStart).
  std::unique_ptr<tracking::digitization::PulseShape> pulse_shape_;

  // -------------------------------------------------------------------------
  // Lorentz angle computation (Mode 1, optional)
  // -------------------------------------------------------------------------
  /// If false, skip Lorentz angle calculation and drift carriers straight
  /// (equivalent to zero magnetic field perpendicular to the sensor normal).
  bool use_lorentz_{true};
  /// Path to the magnetic field map file.  Empty = use fixed configured
  /// tangents.
  std::string field_map_{""};
  /// Per-layer cached Lorentz tangents: layer_id → {tan_electron, tan_hole}.
  std::unordered_map<unsigned int, std::pair<double, double>>
      lorentz_tan_cache_;

  void buildLorentzCache();

  // -------------------------------------------------------------------------
  // Common
  // -------------------------------------------------------------------------
  /// Input collection pass name.
  std::string tracker_hit_passname_;

  /// If non-empty, write a CSV of all ACTS surface transforms to this path.
  std::string dump_geo_csv_{""};

  std::default_random_engine generator_;
  std::shared_ptr<std::normal_distribution<float>> normal_;
};  // DigitizationProcessor

}  // namespace tracking::reco

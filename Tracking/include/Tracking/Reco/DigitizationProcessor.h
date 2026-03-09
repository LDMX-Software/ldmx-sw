#pragma once

#include "Framework/RandomNumberSeedService.h"
#include "Tracking/Reco/TrackingGeometryUser.h"

//--- ACTS ---//
#include "Acts/Definitions/Units.hpp"
#include "Acts/Surfaces/RectangleBounds.hpp"
#include "Acts/Surfaces/Surface.hpp"

//--- LDMX ---//
#include "Tracking/Digitization/PulseShape.h"
#include "Tracking/Digitization/SiStripDigitizer.h"
#include "Tracking/Event/Measurement.h"
#include "Tracking/Event/RawSiStripHit.h"
#include "Tracking/Sim/TrackingUtils.h"

//--- C++ ---//
#include <chrono>
#include <memory>
#include <random>

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
   *                  with one RawSiStripHit per above-threshold readout strip.
   */
  std::vector<ldmx::Measurement> digitizeHits(
      const std::vector<ldmx::SimTrackerHit>& sim_hits,
      std::vector<ldmx::RawSiStripHit>* raw_hits = nullptr);

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
  /// Charge-to-ADC gain [electrons per ADC count].
  double adc_electrons_per_count_{50.0};
  /// ADC pedestal [counts].
  int adc_pedestal_{0};
  /// ADC dynamic range [bits] (e.g. 12 → max 4095 counts).
  int adc_bits_{12};

  // Pulse shaping
  /// Name of the pulse shape: "CRRC" (default) or "FourPole".
  std::string pulse_shape_name_{"CRRC"};
  /// Peaking time tp [ns].
  double tp_{45.0};
  /// Second time constant tp2 [ns] (FourPole only; must satisfy tp > tp2 > 0).
  double tp2_{10.0};
  /// Time of the first ADC sample relative to the hit arrival time [ns].
  double t0_offset_ns_{0.0};
  /// Interval between consecutive ADC samples [ns].
  double sampling_interval_ns_{25.0};
  /// Number of ADC samples per strip hit (fixed at 3 for LDMX).
  static constexpr int N_SAMPLES{3};
  /// The constructed pulse shape (created in onProcessStart).
  std::unique_ptr<tracking::digitization::PulseShape> pulse_shape_;

  // -------------------------------------------------------------------------
  // Common
  // -------------------------------------------------------------------------
  /// Input collection pass name.
  std::string tracker_hit_passname_;

  std::default_random_engine generator_;
  std::shared_ptr<std::normal_distribution<float>> normal_;
};  // DigitizationProcessor

}  // namespace tracking::reco

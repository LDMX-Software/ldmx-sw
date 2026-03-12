#pragma once

#include <map>
#include <random>
#include <utility>

#include "Acts/Definitions/Algebra.hpp"
#include "SimCore/Event/SimTrackerHit.h"
#include "Tracking/Digitization/SiStripConstants.h"

namespace tracking::digitization {

/**
 * Realistic digitization of silicon strip sensor hits.
 *
 * Follows the physics model of CDFSiSensorSim (itself a port of the LCSIM /
 * HPS Java digitization).  For each SimTrackerHit the track segment through
 * the sensor is divided adaptively into sub-segments whose number is chosen
 * so that the U displacement per segment is at most a configurable fraction
 * of the sense pitch.  For each sub-segment:
 *
 *  1. Charge creation:  N_ehp = Edep / E_pair  (E_pair = 3.62 eV for Si).
 *  2. Charge trapping:  linear collection-efficiency penalty proportional to
 *     drift distance (configurable; 0 for unirradiated sensors).
 *  3. Diffusion sigma:  carrier-type-aware trapezoidal-field formula
 *       - minority carriers (e⁻ in p-type bulk, h⁺ in n-type):
 *           σ² = (kT/q)·t²/V_dep · ln(V_sum / (V_sum − 2·V_dep·d/t))
 *       - majority carriers (h⁺ in p-type, e⁻ in n-type):
 *           σ² = (kT/q)·t²/V_dep · ln((ΔV + 2·V_dep·d/t) / ΔV)
 *     where d is the drift distance, V_sum = V_bias+V_dep, ΔV = V_bias−V_dep.
 *     Falls back to uniform-field formula when V_bias ≤ V_dep.
 *  4. Lorentz drift:  charge cloud centroid shifted by d·tan(θ_L) in U;
 *     effective 1-D sigma broadened by 1/cos²(θ_L) = 1+tan²(θ_L).
 *  5. Strip charge integration:  Gaussian integrated over each sense strip
 *     via the error function.
 *  6. Sense→readout transfer:  sense strips (fine pitch) summed into readout
 *     strips (coarser pitch) according to the AC-coupling ratio.
 *
 * Both electron and hole sides can be simulated independently; by default
 * only the electron (n-strip) side is read out, as for a single-sided
 * p-type sensor.
 *
 * Coordinate convention
 * ---------------------
 *   local[0] = U  (precision direction, perpendicular to strips)
 *   local[1] = V  (strip direction)
 *   local[2] = W  (surface normal)
 *
 * Electrons are collected at W = +thickness/2 (the n-strip side).
 * Holes    are collected at W = −thickness/2 (the p-bulk/backplane side).
 *
 * Strip indexing: readout strip m is centred at U = (m − N/2) · readout_pitch,
 *                 where N = n_readout_strips (default 512).
 *                 Strip 0 is at the most-negative U edge; strip N−1 at the
 *                 most-positive U edge.  The sensor centre is between strips
 *                 N/2−1 and N/2.
 * Mapping: sense strip n → readout strip ⌊n / ratio⌋ + N/2
 *          where ratio = round(readout_pitch / sense_pitch).
 */
class SiStripDigitizer {
 public:
  /// All parameters describing one silicon strip sensor layer.
  struct SensorParams {
    /// Sensor thickness [mm].  Must be set from the geometry before use.
    double thickness{0.0};
    /// Sense (inner) electrode pitch [mm].
    double sense_pitch{SENSE_PITCH_MM};
    /// Readout strip pitch [mm].  Must be an integer multiple of sense_pitch.
    double readout_pitch{READOUT_PITCH_MM};
    /// Applied reverse-bias voltage [V].
    double bias_voltage{BIAS_VOLTAGE_V};
    /// Full-depletion voltage [V].
    double depletion_voltage{DEPLETION_VOLTAGE_V};
    /// Operating temperature [K].
    double temperature{TEMPERATURE_K};
    /// Electronic noise sigma [electrons ENC].
    double noise_electrons{NOISE_ELECTRONS};
    /// Readout threshold [electrons].  Strips below this are suppressed.
    double threshold_electrons{THRESHOLD_ELECTRONS};
    /// true = n-type bulk;  false = p-type bulk (default, most SVT sensors).
    bool is_n_type{false};
    /// Simulate and read out the electron-collection side (n-strips).
    bool electron_side_readout{false};
    /// Simulate and read out the hole-collection side (p-strips / backplane).
    bool hole_side_readout{true};
    /// tan(θ_Lorentz) for electrons.  Sign encodes U-shift direction.
    double electron_lorentz_tangent{0.0};
    /// tan(θ_Lorentz) for holes.
    double hole_lorentz_tangent{0.0};
    /// Charge-trapping fraction lost per 100 µm of drift.
    /// 0.0 for unirradiated; ~0.2 is typical at 1×10¹⁵ n_eq/cm².
    double trapping{0.0};
    /// Adaptive segmentation granularity: max U-step as fraction of
    /// sense_pitch.  Number of segments = max(n_segments_min,
    ///   ceil(|path_length · dir_u| / (granularity · sense_pitch))).
    double deposition_granularity{0.10};
    /// Minimum number of track sub-segments (used when the track is close to
    /// normal incidence so the adaptive formula gives a very small count).
    int n_segments_min{5};
    /// Total number of readout strips.  Strip index 0 is at the most-negative
    /// U edge; strip N−1 at the most-positive edge.
    int n_readout_strips{N_READOUT_STRIPS};
  };

  SiStripDigitizer() = default;

  /**
   * @param params     Sensor parameters.
   * @param generator  Random-number engine (owned by caller).
   */
  SiStripDigitizer(const SensorParams& params,
                   std::default_random_engine& generator);

  /**
   * Simulate charge collection for a single hit.
   *
   * Runs both electron and hole sides as configured, performs the
   * sense-to-readout charge transfer, and returns the final readout-strip
   * charge map.
   *
   * @param edep        Energy deposited in the sensitive volume [MeV].
   * @param local_pos   3D hit midpoint in sensor-local coordinates [mm].
   * @param local_dir   Unit track direction in sensor-local coordinates.
   * @param path_length Track path length through the sensor volume [mm].
   *
   * @return Map of readout_strip_index → collected charge [electrons].
   *         Indices are in [0, n_readout_strips); strip 0 is at the
   *         most-negative U edge of the sensor.
   */
  std::map<int, double> computeStripCharges(double edep,
                                             const Acts::Vector3& local_pos,
                                             const Acts::Vector3& local_dir,
                                             double path_length) const;

  /**
   * Add Gaussian electronic noise to signal strips and their immediate
   * neighbours, then remove strips below the readout threshold.
   *
   * @param strip_charges  Readout-strip charge map.  Modified in place.
   */
  void applyNoiseAndThreshold(std::map<int, double>& strip_charges);

  /**
   * Compute a charge-weighted centroid position from a readout-strip cluster.
   *
   * @param strip_charges  Map of readout_strip_index → charge [electrons].
   * @return Pair {centroid_u [mm], resolution_u [mm]}.
   */
  std::pair<double, double> clusterToPosition(
      const std::map<int, double>& strip_charges) const;

  const SensorParams& params() const { return params_; }

  /// Set the sensor thickness [mm] from the geometry before processing hits.
  void setThickness(double thickness) { params_.thickness = thickness; }

 private:
  /**
   * Number of track sub-segments, chosen adaptively so that the U
   * displacement per segment does not exceed
   * deposition_granularity × sense_pitch.
   */
  int adaptiveNSegments(const Acts::Vector3& local_dir,
                        double path_length) const;

  /**
   * Diffusion sigma [mm] for carriers drifting distance @p d [mm].
   *
   * @param d           Drift distance [mm].
   * @param is_minority True for minority carriers (e⁻ in p-type, h⁺ in
   *                    n-type); false for majority carriers.
   */
  double diffusionSigma(double d, bool is_minority) const;

  /**
   * Fraction of a Gaussian charge cloud (centred at @p u0 with sigma
   * @p sigma) collected by a strip of width @p pitch centred at
   * @p strip_center.
   */
  double stripFraction(double u0, double sigma, double strip_center,
                       double pitch) const;

  /**
   * Simulate one carrier type and return the sense-strip charge map.
   *
   * @param q_per_seg     Charge per sub-segment [electrons].
   * @param n_seg         Number of sub-segments.
   * @param local_pos     3D hit midpoint in local coordinates [mm].
   * @param local_dir     Unit track direction in local coordinates.
   * @param path_length   Path length through sensor [mm].
   * @param w_collect     W coordinate of collection electrode [mm].
   * @param lorentz_tan   tan(θ_Lorentz) for this carrier (may be 0).
   * @param is_minority   True if this carrier is the minority species.
   */
  std::map<int, double> computeCarrierCharges(double q_per_seg, int n_seg,
                                               const Acts::Vector3& local_pos,
                                               const Acts::Vector3& local_dir,
                                               double path_length,
                                               double w_collect,
                                               double lorentz_tan,
                                               bool is_minority) const;

  /**
   * Sum sense-strip charges into readout-strip charges according to the
   * AC-coupling ratio  ratio = round(readout_pitch / sense_pitch).
   * Sense strip n maps to readout strip floor(n / ratio).
   */
  std::map<int, double> senseToReadout(
      const std::map<int, double>& sense_charges) const;

  SensorParams params_;
  std::default_random_engine* generator_{nullptr};
  mutable std::normal_distribution<double> normal_{0.0, 1.0};
};

}  // namespace tracking::digitization

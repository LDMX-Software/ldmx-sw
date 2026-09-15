#pragma once

#include <vector>

#include "Tracking/Digitization/PulseShape.h"

namespace tracking::digitization {

/**
 * Fits a pulse shape to the ADC samples of a single silicon-strip readout
 * channel to extract hit amplitude and time.
 *
 * Algorithm
 * ---------
 * With N ADC samples and 2 free parameters (amplitude A and hit time T), the
 * chi-squared is:
 *
 *   χ²(T) = Σ_i  [(S_i − ped − A(T)·f(t_i − T)) / σ]²
 *
 * where t_i = t0_offset + i·Δt are the sample times and f is the pulse shape.
 * For each candidate T, A is solved analytically:
 *
 *   A(T) = Σ_i (S_i − ped)·f(t_i − T) / Σ_i f(t_i − T)²
 *
 * A 1-D scan over T followed by a parabolic refinement near the minimum
 * gives the fitted values. This is O(n_scan × N_samples) per strip and
 * runs in < 1 µs.
 *
 * The fit result t0 is defined as T − t0_offset, i.e. the hit arrival time
 * relative to sample 0.  A value of t0 = tp (peaking time) means the pulse
 * peak coincides with sample 0.
 *
 * Timing convention
 * -----------------
 *   Sample i is taken at time  t_i = t0_offset_ns + i · sampling_interval_ns
 *   The pulse shape is evaluated at  f(t_i − T)  where T is the absolute hit
 *   arrival time in the same frame.  The output t0 = T is stored directly so
 *   the caller can subtract their own reference as needed.
 */
class StripPulseFitter {
 public:
  // I like not having underscores for struct members,
  // but that's not settable until a later version of clang-tidy
  // NOLINTBEGIN(readability-identifier-naming)
  struct FitResult {
    double amplitude{
        0};          ///< Fitted peak amplitude [ADC counts], ped-subtracted.
    double t0{0};    ///< Fitted hit arrival time T [ns].
    double chi2{0};  ///< Chi-squared value at the minimum.
    int ndf{0};      ///< Degrees of freedom = n_samples − 2.
    bool converged{false};  ///< False if no above-pedestal samples found.
  };
  // NOLINTEND(readability-identifier-naming)

  /**
   * @param shape               Pulse shape function (owned externally).
   * @param t0_offset_ns        Time of sample 0 in the hit-time frame [ns].
   * @param sampling_interval_ns  Interval between samples [ns].
   * @param pedestal_adc        Per-sample pedestal [ADC counts].
   * @param noise_sigma_adc     Per-sample noise RMS [ADC counts], used for χ².
   * @param t_scan_min_ns       Lower bound of the T scan range [ns].
   * @param t_scan_max_ns       Upper bound of the T scan range [ns].
   * @param t_scan_step_ns      Step size of the coarse T scan [ns].
   */
  StripPulseFitter(const PulseShape& shape, double t0_offset_ns,
                   double sampling_interval_ns, double pedestal_adc,
                   double noise_sigma_adc, double t_scan_min_ns = -50.0,
                   double t_scan_max_ns = 150.0, double t_scan_step_ns = 1.0);

  /**
   * Fit the pulse to the given ADC sample vector.
   *
   * @param samples  ADC values (one per sample, any integer type cast to
   * short).
   */
  FitResult fit(const std::vector<short>& samples) const;

 private:
  /// Evaluate χ²(T) and the corresponding best-fit amplitude.
  std::pair<double, double> evalAtT(const std::vector<short>& samples,
                                    double T) const;

  const PulseShape* shape_;
  double t0_offset_ns_;
  double sampling_interval_ns_;
  double pedestal_adc_;
  double inv_sigma2_;  ///< 1/σ², pre-computed.
  double t_scan_min_ns_;
  double t_scan_max_ns_;
  double t_scan_step_ns_;
};

}  // namespace tracking::digitization

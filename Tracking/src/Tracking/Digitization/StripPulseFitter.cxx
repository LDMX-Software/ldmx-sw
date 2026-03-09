#include "Tracking/Digitization/StripPulseFitter.h"

#include <cmath>
#include <limits>

namespace tracking::digitization {

StripPulseFitter::StripPulseFitter(const PulseShape& shape,
                                   double t0_offset_ns,
                                   double sampling_interval_ns,
                                   double pedestal_adc,
                                   double noise_sigma_adc,
                                   double t_scan_min_ns,
                                   double t_scan_max_ns,
                                   double t_scan_step_ns)
    : shape_(&shape),
      t0_offset_ns_(t0_offset_ns),
      sampling_interval_ns_(sampling_interval_ns),
      pedestal_adc_(pedestal_adc),
      inv_sigma2_(1.0 / (noise_sigma_adc * noise_sigma_adc)),
      t_scan_min_ns_(t_scan_min_ns),
      t_scan_max_ns_(t_scan_max_ns),
      t_scan_step_ns_(t_scan_step_ns) {}

// ---------------------------------------------------------------------------
// Private helper: chi2 and amplitude for a fixed hit time T
// ---------------------------------------------------------------------------

std::pair<double, double> StripPulseFitter::evalAtT(
    const std::vector<short>& samples, double T) const {

  const int n = static_cast<int>(samples.size());

  // Evaluate pulse shape at each sample time.
  // t_i = t0_offset + i * dt  →  argument to shape = t_i - T
  double sum_sf  = 0.0;  // Σ (S_i - ped) * f_i
  double sum_f2  = 0.0;  // Σ f_i²

  for (int i = 0; i < n; ++i) {
    const double t_arg = t0_offset_ns_ + i * sampling_interval_ns_ - T;
    const double fi    = shape_->eval(t_arg);
    const double si    = static_cast<double>(samples[i]) - pedestal_adc_;
    sum_sf += si * fi;
    sum_f2 += fi * fi;
  }

  // Degenerate: pulse shape is zero everywhere in the window.
  if (sum_f2 < 1.0e-12) {
    return {std::numeric_limits<double>::max(), 0.0};
  }

  const double amplitude = sum_sf / sum_f2;

  // Compute chi2 for this (T, amplitude) pair.
  double chi2 = 0.0;
  for (int i = 0; i < n; ++i) {
    const double t_arg  = t0_offset_ns_ + i * sampling_interval_ns_ - T;
    const double fi     = shape_->eval(t_arg);
    const double si     = static_cast<double>(samples[i]) - pedestal_adc_;
    const double resid  = si - amplitude * fi;
    chi2 += resid * resid * inv_sigma2_;
  }

  return {chi2, amplitude};
}

// ---------------------------------------------------------------------------
// Public: fit
// ---------------------------------------------------------------------------

StripPulseFitter::FitResult StripPulseFitter::fit(
    const std::vector<short>& samples) const {

  FitResult result;
  result.ndf = static_cast<int>(samples.size()) - 2;

  // Quick sanity: check that at least one sample is above pedestal.
  bool any_signal = false;
  for (auto s : samples) {
    if (static_cast<double>(s) > pedestal_adc_) { any_signal = true; break; }
  }
  if (!any_signal) {
    return result;  // converged = false
  }

  // -------------------------------------------------------------------
  // Coarse scan over T to find the approximate minimum.
  // -------------------------------------------------------------------
  double best_chi2 = std::numeric_limits<double>::max();
  double best_T    = t_scan_min_ns_;
  double best_amp  = 0.0;

  // Keep track of the three-point neighbourhood for parabolic refinement.
  double T_lo = t_scan_min_ns_, chi2_lo = std::numeric_limits<double>::max();
  double T_hi = t_scan_min_ns_, chi2_hi = std::numeric_limits<double>::max();

  double T_prev = t_scan_min_ns_;
  double chi2_prev = std::numeric_limits<double>::max();

  for (double T = t_scan_min_ns_; T <= t_scan_max_ns_;
       T += t_scan_step_ns_) {
    auto [chi2, amp] = evalAtT(samples, T);

    if (chi2 < best_chi2) {
      // Update neighbourhood: the previous point becomes T_lo.
      T_lo    = T_prev;
      chi2_lo = chi2_prev;
      best_chi2 = chi2;
      best_T    = T;
      best_amp  = amp;
    } else if (T > best_T && chi2_hi > best_chi2) {
      // First point to the right of the minimum.
      T_hi    = T;
      chi2_hi = chi2;
    }

    T_prev    = T;
    chi2_prev = chi2;
  }

  // -------------------------------------------------------------------
  // Parabolic refinement around the minimum if neighbours are available.
  // -------------------------------------------------------------------
  if (T_lo < best_T && T_hi > best_T
      && chi2_lo > best_chi2 && chi2_hi > best_chi2) {
    // Fit a parabola through (T_lo, chi2_lo), (best_T, best_chi2), (T_hi, chi2_hi).
    const double d1  = best_T  - T_lo;
    const double d2  = T_hi    - best_T;
    const double dc1 = best_chi2 - chi2_lo;
    const double dc2 = chi2_hi   - best_chi2;
    // Vertex of parabola: ΔT = (d1²·dc2 - d2²·dc1) / (2·(d1·dc2 + d2·dc1))
    const double denom = 2.0 * (d1 * dc2 + d2 * dc1);
    if (std::abs(denom) > 1.0e-12) {
      const double delta = (d1 * d1 * dc2 - d2 * d2 * dc1) / denom;
      // Only accept the refinement if it stays within the bracket.
      if (std::abs(delta) < std::max(d1, d2)) {
        const double T_refined = best_T + delta;
        auto [chi2_ref, amp_ref] = evalAtT(samples, T_refined);
        if (chi2_ref < best_chi2) {
          best_T   = T_refined;
          best_amp = amp_ref;
          best_chi2 = chi2_ref;
        }
      }
    }
  }

  result.amplitude = best_amp;
  result.t0        = best_T;
  result.chi2      = best_chi2;
  result.converged = true;
  return result;
}

}  // namespace tracking::digitization

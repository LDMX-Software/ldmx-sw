#include "Tools/PulseFitter.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <tuple>
#include <vector>

namespace ldmx {

namespace {
// empty mask: every sample is used
bool isUsed(const std::vector<bool>& used, std::size_t i) {
  return used.empty() || used.at(i);
}

int nUsed(const std::vector<short>& samples, const std::vector<bool>& used) {
  int n = 0;
  for (std::size_t i = 0; i < samples.size(); ++i) n += isUsed(used, i);
  return n;
}
}  // namespace

PulseFitter::PulseFitter(const PulseShape& shape, double t0_offset_ns,
                         double sampling_interval_ns, double pedestal_adc,
                         double noise_sigma_adc, double t_scan_min_ns,
                         double t_scan_max_ns, double t_scan_step_ns)
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

std::pair<double, double> PulseFitter::evalAtT(
    const std::vector<short>& samples, double T,
    const std::vector<bool>& used) const {
  const int n = static_cast<int>(samples.size());

  // Evaluate pulse shape at each sample time.
  // t_i = t0_offset + i * dt  →  argument to shape = t_i - T
  double sum_sf = 0.0;  // Σ (S_i - ped) * f_i
  double sum_f2 = 0.0;  // Σ f_i²

  for (int i = 0; i < n; ++i) {
    if (!isUsed(used, i)) continue;
    const double t_arg = t0_offset_ns_ + i * sampling_interval_ns_ - T;
    const double fi = shape_->eval(t_arg);
    const double si = static_cast<double>(samples[i]) - pedestal_adc_;
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
    if (!isUsed(used, i)) {
      // suppressed sample: penalize a model above the ZS threshold
      if (zs_threshold_adc_ > 0.) {
        const double t_arg = t0_offset_ns_ + i * sampling_interval_ns_ - T;
        const double over = amplitude * shape_->eval(t_arg) - zs_threshold_adc_;
        if (over > 0.) chi2 += over * over * inv_sigma2_;
      }
      continue;
    }
    const double t_arg = t0_offset_ns_ + i * sampling_interval_ns_ - T;
    const double fi = shape_->eval(t_arg);
    const double si = static_cast<double>(samples[i]) - pedestal_adc_;
    const double resid = si - amplitude * fi;
    chi2 += resid * resid * inv_sigma2_;
  }

  return {chi2, amplitude};
}

// ---------------------------------------------------------------------------
// Public: fit
// ---------------------------------------------------------------------------

PulseFitter::FitResult PulseFitter::fit(const std::vector<short>& samples,
                                        const std::vector<bool>& used) const {
  FitResult result;
  result.ndf = nUsed(samples, used) - 2;

  // Quick sanity: check that at least one sample is above pedestal.
  bool any_signal = false;
  for (std::size_t i = 0; i < samples.size(); ++i) {
    if (isUsed(used, i) && static_cast<double>(samples[i]) > pedestal_adc_) {
      any_signal = true;
      break;
    }
  }
  if (!any_signal) {
    return result;  // converged = false
  }

  // -------------------------------------------------------------------
  // Coarse scan over T to find the approximate minimum.
  // -------------------------------------------------------------------
  std::vector<double> scan_t, scan_chi2, scan_amp;
  for (double t = t_scan_min_ns_; t <= t_scan_max_ns_; t += t_scan_step_ns_) {
    auto [chi2, amp] = evalAtT(samples, t, used);
    scan_t.push_back(t);
    scan_chi2.push_back(chi2);
    scan_amp.push_back(amp);
  }
  if (scan_t.empty()) {
    return result;  // converged = false
  }

  const std::size_t i_best = static_cast<std::size_t>(
      std::min_element(scan_chi2.begin(), scan_chi2.end()) - scan_chi2.begin());
  double best_chi2 = scan_chi2[i_best];
  double best_t = scan_t[i_best];
  double best_amp = scan_amp[i_best];

  // Masked fit with a flat minimum: T is not constrained, e.g. a single kept
  // sample fits exactly for any T where the suppressed samples stay below
  // threshold. Take the middle of the flat range; its first point would
  // give the largest amplitude the threshold allows. Full-sample fits keep
  // the first minimum.
  std::size_t i_lo = i_best, i_hi = i_best;
  const bool masked = nUsed(samples, used) < static_cast<int>(samples.size());
  if (masked && best_chi2 < std::numeric_limits<double>::max()) {
    const double flat = best_chi2 + 1.0e-6 * (1.0 + best_chi2);
    while (i_lo > 0 && scan_chi2[i_lo - 1] <= flat) --i_lo;
    while (i_hi + 1 < scan_t.size() && scan_chi2[i_hi + 1] <= flat) ++i_hi;
  }
  if (i_hi > i_lo) {
    best_t = 0.5 * (scan_t[i_lo] + scan_t[i_hi]);
    std::tie(best_chi2, best_amp) = evalAtT(samples, best_t, used);
  } else if (i_best > 0 && i_best + 1 < scan_t.size() &&
             scan_chi2[i_best - 1] > best_chi2 &&
             scan_chi2[i_best + 1] > best_chi2) {
    // -----------------------------------------------------------------
    // Parabolic refinement around the minimum.
    // -----------------------------------------------------------------
    const double t_lo = scan_t[i_best - 1], chi2_lo = scan_chi2[i_best - 1];
    const double t_hi = scan_t[i_best + 1], chi2_hi = scan_chi2[i_best + 1];
    // Fit a parabola through (T_lo, chi2_lo), (best_T, best_chi2), (T_hi,
    // chi2_hi).
    const double d1 = best_t - t_lo;
    const double d2 = t_hi - best_t;
    const double dc1 = chi2_lo - best_chi2;
    const double dc2 = chi2_hi - best_chi2;
    // Vertex of parabola: ΔT = (d2²·dc1 - d1²·dc2) / (2·(d1·dc2 + d2·dc1))
    const double denom = 2.0 * (d1 * dc2 + d2 * dc1);
    if (std::abs(denom) > 1.0e-12) {
      const double delta = (d2 * d2 * dc1 - d1 * d1 * dc2) / denom;
      // Only accept the refinement if it stays within the bracket.
      if (std::abs(delta) < std::max(d1, d2)) {
        const double t_refined = best_t + delta;
        auto [chi2_ref, amp_ref] = evalAtT(samples, t_refined, used);
        if (chi2_ref < best_chi2) {
          best_t = t_refined;
          best_amp = amp_ref;
          best_chi2 = chi2_ref;
        }
      }
    }
  }

  result.amplitude = best_amp;
  result.t0 = best_t;
  result.chi2 = best_chi2;
  result.converged = true;
  return result;
}

// ---------------------------------------------------------------------------
// Public: fitFixed
// ---------------------------------------------------------------------------

PulseFitter::FitResult PulseFitter::fitFixed(
    const std::vector<short>& samples, double T,
    const std::vector<bool>& used) const {
  FitResult result;
  result.ndf = nUsed(samples, used) - 1;
  auto [chi2, amp] = evalAtT(samples, T, used);
  result.amplitude = amp;
  result.t0 = T;
  result.chi2 = chi2;
  result.converged = chi2 < std::numeric_limits<double>::max();
  return result;
}

}  // namespace ldmx

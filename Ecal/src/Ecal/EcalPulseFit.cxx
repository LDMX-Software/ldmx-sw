#include "Ecal/EcalPulseFit.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace ecal {

EcalPulseFit::EcalPulseFit(const std::vector<double>& shape,
                           double clock_cycle)
    : clock_cycle_{clock_cycle} {
  if (shape.size() != 5) {
    throw std::invalid_argument("EcalPulseFit needs 5 pulse shape parameters");
  }
  rate_up_ = shape[0];
  time_up_ = shape[1];
  rate_dn_ = shape[2];
  time_dn_ = shape[3];
  time_peak_ = shape[4];
  // same form as the HgcrocEmulator pulse
  norm_ = (1. + std::exp(rate_up_ * (time_peak_ - time_up_))) *
          (1. + std::exp(rate_dn_ * (time_peak_ - time_dn_)));
}

double EcalPulseFit::shape(double t) const {
  return norm_ / ((1. + std::exp(rate_up_ * (t - time_up_ + time_peak_))) *
                  (1. + std::exp(rate_dn_ * (t - time_dn_ + time_peak_))));
}

EcalPulseFit::Result EcalPulseFit::fitAt(const std::vector<double>& adc,
                                         double pedestal, int i_soi, int first,
                                         int last, double t0) const {
  double sum_yf{0.}, sum_ff{0.};
  for (int i{first}; i < last; ++i) {
    double f = shape((i - i_soi) * clock_cycle_ - t0);
    sum_yf += (adc[i] - pedestal) * f;
    sum_ff += f * f;
  }
  Result r;
  r.t0 = t0;
  r.amplitude = sum_ff > 0. ? sum_yf / sum_ff : 0.;
  for (int i{first}; i < last; ++i) {
    double d = adc[i] - pedestal -
               r.amplitude * shape((i - i_soi) * clock_cycle_ - t0);
    r.chi2 += d * d;
  }
  return r;
}

EcalPulseFit::Result EcalPulseFit::fit(const std::vector<double>& adc,
                                       double pedestal, int i_soi, int first,
                                       int n) const {
  const int n_adc = static_cast<int>(adc.size());
  // keep a full window inside the digi
  first = std::clamp(first, 0, n > 0 ? std::max(n_adc - n, 0) : 0);
  const int last = n > 0 ? std::min(first + n, n_adc) : n_adc;
  if (last <= first) return Result{};

  // peak within half a clock of the window, else noise tails fake big pulses
  const double t_lo = (first - i_soi - 0.5) * clock_cycle_;
  const double t_hi = (last - 1 - i_soi + 0.5) * clock_cycle_;

  Result best;
  bool found{false};
  for (double t0{t_lo}; t0 <= t_hi; t0 += 1.) {
    auto r = fitAt(adc, pedestal, i_soi, first, last, t0);
    if (r.amplitude > 0. and (not found or r.chi2 < best.chi2)) {
      best = r;
      found = true;
    }
  }
  if (not found) return fitAt(adc, pedestal, i_soi, first, last, 0.);

  const double t_c = best.t0;
  for (double t0{std::max(t_c - 1., t_lo)}; t0 <= std::min(t_c + 1., t_hi);
       t0 += 0.05) {
    auto r = fitAt(adc, pedestal, i_soi, first, last, t0);
    if (r.amplitude > 0. and r.chi2 < best.chi2) best = r;
  }
  return best;
}

EcalPulseFit::Result EcalPulseFit::fitFixed(const std::vector<double>& adc,
                                            double pedestal, int i_soi,
                                            double t0, int n) const {
  const int n_adc = static_cast<int>(adc.size());
  int first{0};
  if (n > 0) {
    int i_peak = i_soi + static_cast<int>(std::lround(t0 / clock_cycle_));
    first = std::clamp(i_peak - 1, 0, std::max(n_adc - n, 0));
  }
  const int last = n > 0 ? std::min(first + n, n_adc) : n_adc;
  return fitAt(adc, pedestal, i_soi, first, last, t0);
}

}  // namespace ecal

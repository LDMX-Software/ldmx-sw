#include "Tools/PulseShape.h"

#include "Framework/Exception/Exception.h"

namespace ldmx {

// ---------------------------------------------------------------------------
// FourPoleShape
// ---------------------------------------------------------------------------

FourPoleShape::FourPoleShape(double tp, double tp2) : tp_(tp), tp2_(tp2) {
  if (tp2_ <= 0.0 || tp_ <= tp2_) {
    EXCEPTION_RAISE("InvalidArgument", "FourPoleShape: requires tp > tp2 > 0");
  }

  a_ = (tp_ * tp_) / std::pow(tp_ - tp2_, 3.0);
  b_ = (tp_ - tp2_) / (tp_ * tp2_);

  // Approximate peak time: 3·(tp·tp2³)^(1/4)
  const double t_peak = 3.0 * std::pow(tp_ * std::pow(tp2_, 3.0), 0.25);

  // Evaluate un-normalised amplitude at the peak for normalisation.
  peak_amp_ = a_ * (std::exp(-t_peak / tp_) -
                    std::exp(-t_peak / tp2_) *
                        (1.0 + t_peak * b_ + 0.5 * t_peak * t_peak * b_ * b_));
}

double FourPoleShape::eval(double t) const {
  if (t <= 0.0) return 0.0;
  const double g =
      a_ * (std::exp(-t / tp_) -
            std::exp(-t / tp2_) * (1.0 + t * b_ + 0.5 * t * t * b_ * b_));
  return g / peak_amp_;
}

// ---------------------------------------------------------------------------
// Factory
// ---------------------------------------------------------------------------

std::unique_ptr<PulseShape> PulseShape::make(const std::string& name, double tp,
                                             double tp2) {
  if (name == "CRRC") {
    return std::make_unique<CRRCShape>(tp);
  } else if (name == "FourPole") {
    return std::make_unique<FourPoleShape>(tp, tp2);
  }
  EXCEPTION_RAISE("InvalidArgument", "PulseShape::make: unknown shape '" +
                                         name + "'. Use 'CRRC' or 'FourPole'.");
}

// ---------------------------------------------------------------------------
// HgcrocShape
// ---------------------------------------------------------------------------

HgcrocShape::HgcrocShape(double rate_up, double time_up, double rate_dn,
                         double time_dn, double time_peak)
    : rate_up_(rate_up),
      time_up_(time_up),
      rate_dn_(rate_dn),
      time_dn_(time_dn),
      time_peak_(time_peak) {
  norm_ = (1.0 + std::exp(rate_up_ * (time_peak_ - time_up_))) *
          (1.0 + std::exp(rate_dn_ * (time_peak_ - time_dn_)));
}

double HgcrocShape::eval(double t) const {
  return norm_ / ((1.0 + std::exp(rate_up_ * (t - time_up_ + time_peak_))) *
                  (1.0 + std::exp(rate_dn_ * (t - time_dn_ + time_peak_))));
}

}  // namespace ldmx

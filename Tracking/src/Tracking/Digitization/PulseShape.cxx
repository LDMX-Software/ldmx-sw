#include "Tracking/Digitization/PulseShape.h"

#include <stdexcept>

namespace tracking::digitization {

// ---------------------------------------------------------------------------
// FourPoleShape
// ---------------------------------------------------------------------------

FourPoleShape::FourPoleShape(double tp, double tp2)
    : tp_(tp), tp2_(tp2) {

  if (tp2_ <= 0.0 || tp_ <= tp2_) {
    throw std::invalid_argument(
        "FourPoleShape: requires tp > tp2 > 0");
  }

  A_ = (tp_ * tp_) / std::pow(tp_ - tp2_, 3.0);
  B_ = (tp_ - tp2_) / (tp_ * tp2_);

  // Approximate peak time: 3·(tp·tp2³)^(1/4)
  const double t_peak = 3.0 * std::pow(tp_ * std::pow(tp2_, 3.0), 0.25);

  // Evaluate un-normalised amplitude at the peak for normalisation.
  peak_amp_ = A_ * (std::exp(-t_peak / tp_)
                    - std::exp(-t_peak / tp2_)
                          * (1.0 + t_peak * B_
                             + 0.5 * t_peak * t_peak * B_ * B_));
}

double FourPoleShape::eval(double t) const {
  if (t <= 0.0) return 0.0;
  const double g = A_ * (std::exp(-t / tp_)
                         - std::exp(-t / tp2_)
                               * (1.0 + t * B_ + 0.5 * t * t * B_ * B_));
  return g / peak_amp_;
}

// ---------------------------------------------------------------------------
// Factory
// ---------------------------------------------------------------------------

std::unique_ptr<PulseShape> PulseShape::make(const std::string& name,
                                              double tp, double tp2) {
  if (name == "CRRC") {
    return std::make_unique<CRRCShape>(tp);
  } else if (name == "FourPole") {
    return std::make_unique<FourPoleShape>(tp, tp2);
  }
  throw std::invalid_argument("PulseShape::make: unknown shape '" + name +
                              "'. Use 'CRRC' or 'FourPole'.");
}

}  // namespace tracking::digitization

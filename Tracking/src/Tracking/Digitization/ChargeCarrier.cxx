#include "Tracking/Digitization/ChargeCarrier.h"

#include <math.h>

namespace tracking {
namespace digitization {

double ChargeCarrier::mu0(double temperature) {
  return mu_0_factor_ * std::pow((temperature / TCOEFF), mu_0_exponent_);
}

double ChargeCarrier::muMin(double temperature) {
  return mu_min_factor_ * std::pow((temperature / TCOEFF), mu_min_exponent_);
}

double ChargeCarrier::nRef(double temperature) {
  return N_ref_factor_ * std::pow((temperature / TCOEFF), N_ref_exponent_);
}

double ChargeCarrier::alpha(double temperature) {
  return alpha_factor_ * std::pow((temperature / TCOEFF), alpha_exponent_);
}

}  // namespace digitization
}  // namespace tracking

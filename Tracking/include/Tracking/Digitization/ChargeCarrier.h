#pragma once

#include <stdexcept>

/*
 * Original Author T. K. Nelson
 */

namespace tracking {
namespace digitization {

static const double TCOEFF = 300.0;

class ChargeCarrier {
 public:
  ChargeCarrier(int charge, double mu_0_factor, double mu_0_exponent,
                double mu_min_factor, double mu_min_exponent,
                double N_ref_factor, double N_ref_exponent, double alpha_factor,
                double alpha_exponent) {
    charge_ = charge;
    mu_0_factor_ = mu_0_factor;
    mu_0_exponent_ = mu_0_exponent;
    mu_min_factor_ = mu_min_factor;
    mu_min_exponent_ = mu_min_exponent;
    N_ref_factor_ = N_ref_factor;
    N_ref_exponent_ = N_ref_exponent;
    alpha_factor_ = alpha_factor;
    alpha_exponent_ = alpha_exponent;
  }

  int charge() { return charge_; }

  double mu0(double temperature);
  double muMin(double temperature);
  double nRef(double temperature);
  double alpha(double temperature);

 private:
  int charge_;
  double mu_0_factor_;
  double mu_0_exponent_;
  double mu_min_factor_;
  double mu_min_exponent_;
  double N_ref_factor_;
  double N_ref_exponent_;
  double alpha_factor_;
  double alpha_exponent_;
};

static const ChargeCarrier electron(-1, 1268.0, -2.33, 92.0, -0.57, 1.3E+17,
                                    2.4, 0.91, -0.146);
static const ChargeCarrier hole(1, 406.9, -2.23, 54.3, -0.57, 2.35E+17, 2.4,
                                0.88, -0.146);

static ChargeCarrier getCarrier(int charge) {
  if (charge == -1)
    return electron;
  else if (charge == 1)
    return hole;
  else
    throw std::invalid_argument("No ChargeCarrier for charge specified");
}

}  // namespace digitization
}  // namespace tracking

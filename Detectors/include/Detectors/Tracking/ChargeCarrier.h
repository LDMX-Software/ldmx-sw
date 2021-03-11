#ifndef DETECTORS_TRACKER_CHARGECARRIERS_H
#define DETECTORS_TRACKER_CHARGECARRIERS_H

//---< C++ >---//
#include <cmath>

namespace detectors {
namespace tracker {

/**
 * Class used to describe a charge carrier (Electron or Hole).
 *
 * The properties of each charge carrier were extracted from best fits to
 * the ohmic mobility as a function of impurity concentration (see "A review of
 * some charge transport properties of silicon" by C. Jacoboni et al.) and drift
 * velocity as a function of electric field (see LHCb note 2003-159) of
 * electrons and holes.
 */
class ChargeCarrier {
public:
  /**
   * Constructor.
   *
   * All parameters below except for the charge were extracted from fits as
   * noted in the papers above.
   *
   * @param charge The charge of this carrier
   * @param mu_0_factor Mobility parameter extracted from fit.
   * @param mu_0_exp Mobility parameter extracted from fit.
   * @param mu_min_factor Mobility parameter extracted from fit.
   * @param mu_min_exp Mobility parmeter extracted from fit.
   * @param n_ref_factor Impurity parameter
   * @param n_ref_exp Impurity parameter
   * @param alpha_factor Mobility parameter extracted from fit.
   * @param alpha_exp Mobility parameter extracted from fit.
   */
  ChargeCarrier(int charge, double mu_0_factor, double mu_0_exp,
                double mu_min_factor, double mu_min_exp, double n_ref_factor,
                double n_ref_exp, double alpha_factor, double alpha_exp)
      : charge_(charge), mu_0_factor_(mu_0_factor), mu_0_exp_(mu_0_exp),
        mu_min_factor_(mu_min_factor), mu_min_exp_(mu_min_exp),
        n_ref_factor_(n_ref_factor), n_ref_exp_(n_ref_exp),
        alpha_factor_(alpha_factor), alpha_exp_(alpha_exp){};

  /// Default destructor
  ~ChargeCarrier() = default;

  /// @return The charge of this carrier
  int charge() { return charge_; }

  /// @return The mobility parameter mu_0 at the temperature T [K]
  double mu0(double temperature) {
    return mu_0_factor_ * pow(temperature / 300.0, mu_0_exp_);
  }

  ///  @return The mobility parameter mu_min at the temperature T [K]
  double muMin(double temperature) {
    return mu_min_factor_ * pow(temperature / 300.0, mu_min_exponenent_);
  }

  /// @return The impurity factor at the given temperature T [K]
  double nRef(double temperature) {
    return n_ref_factor_ * pow(temperature / 300.0, n_ref_exp_);
  }

  /// @return The alpha parameter at the given temperature T [K]
  double alpha(double temperature) {
    return alpha_factor_ * pow(temperature / 300.0, alpha_exp_);
  }

private:
  /// Charge The charge of this carrier
  int charge_;
  
  /// Mobility parameter extracted from fit.
  double mu_0_factor_;
  
  /// Mobility parameter extracted from fit.
  double mu_0_exp_;
  
  /// Mobility parameter extracted from fit.
  double mu_min_factor_;
  
  /// Mobility parameter extracted from fit.
  double mu_min_exp_;
  
  /// Impurity parameter
  double n_ref_factor_;
  
  /// Impurity parameter
  double n_ref_exp_;
 
  /// Mobility parameter 
  double alpha_factor_;
  
  /// Mobility parameter 
  double alpha_exp_;
};

} // namespace tracker
} // namespace detectors

#endif // DETECTORS_TRACKER_CHARGECARRIERS_H

/**
 * @file EcalPulseFit.h
 * @brief Template fit of HGCROC ADC samples for amplitude and time
 */

#ifndef ECAL_ECALPULSEFIT_H_
#define ECAL_ECALPULSEFIT_H_

#include <vector>

namespace ecal {

/**
 * Fit ADC samples with the HGCROC pulse shape, amplitude and peak time free.
 *
 * Sample i is modeled as pedestal + A * f((i - i_soi) * clock - t0), where f
 * is the emulator pulse shape normalized to one at zero. For a fixed t0 the
 * best A is linear, so t0 is scanned on a coarse then a fine grid.
 */
class EcalPulseFit {
 public:
  /// Fit output, amplitude in ADC above pedestal and t0 in ns from the SOI
  struct Result {
    double amplitude{0.};
    double t0{0.};
    double chi2{0.};
  };

  /**
   * @param[in] shape {rate_up, time_up, rate_dn, time_dn, time_peak}
   * @param[in] clock_cycle sample spacing [ns]
   */
  EcalPulseFit(const std::vector<double>& shape, double clock_cycle);

  /// Pulse shape normalized to one at t = 0
  double shape(double t) const;

  /**
   * Fit samples [first, first + n) of adc, all of them if n <= 0.
   *
   * @param[in] adc ADC values of all samples
   * @param[in] pedestal ADC pedestal
   * @param[in] i_soi index of the sample of interest
   * @param[in] first index of the first sample in the fit
   * @param[in] n number of samples in the fit
   */
  Result fit(const std::vector<double>& adc, double pedestal, int i_soi,
             int first = 0, int n = 0) const;

  /// Amplitude only fit with t0 known, window starts one sample before t0
  Result fitFixed(const std::vector<double>& adc, double pedestal, int i_soi,
                  double t0, int n = 0) const;

 private:
  /// best amplitude and its chi2 for a fixed t0
  Result fitAt(const std::vector<double>& adc, double pedestal, int i_soi,
               int first, int last, double t0) const;

  double rate_up_, time_up_, rate_dn_, time_dn_, time_peak_;
  double clock_cycle_;
  /// shape normalization so that shape(0) = 1
  double norm_;
};

}  // namespace ecal

#endif  // ECAL_ECALPULSEFIT_H_

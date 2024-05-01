#ifndef TRIGSCINT_SIMQIE_H
#define TRIGSCINT_SIMQIE_H

#include <iostream>
#include "TMath.h"
#include "TRandom3.h"
#include "TrigScint/QIEInputPulse.h"

namespace trigscint {

/**
 * @class SimQIE
 * @brief class for simulating QIE chip output
 * @note This should be initialized only once per simulation
 */
class SimQIE {
 public:
  /**
   * Defaut constructor
   * @note if used, noise & pedestal are not simulated
   */
  SimQIE();

  /**
   * Main constructor
   * @param pd pedestal value
   * @param sg noise value
   * @param seed random seed for noise generation
   */
  SimQIE(float pd, float sg, uint64_t seed);

  /**
   * Set current threshold for TDC latch
   *
   * @param gg = gain
   */
  void setTDCThreshold(const float thr) { tdc_thr_ = thr; }

  /**
   * Set gain of SiPM
   *
   * @param gg = gain
   * @note The multiplicated factor 16e-5 is used to convert from 1.6e-19 to fC
   */
  void setGain(const float gg = 1e+6) { gain_ = gg * 16e-5; }

  /**
   * Set sampling frequency of QIE
   *
   * @param sf = sampling frequency in MHz
   */
  void setFreq(const float sf = 40) { tau_ = 1000 / sf; } 

  /**
   * Set the number of time samples to analyze. 
   *
   * @param maxts No. of time samples to analyze
   */
  void setNTimeSamples(const int maxts = 5) { maxts_ = maxts; }

  /**
   * Quantization error
   */
  float QErr(float Q);

  /**
   * Digitizing input charge
   * @param Charge = The charge to be digitized
   * @note default charge unit = femto C = 1e-15 C
   */
  int Q2ADC(float Charge);

  /**
   * Converting ADC back to charge
   * @param ADC = ADC count
   */
  float ADC2Q(int ADC);

  /**
   * TDC of the input pulse
   * @param pp = pointer to a pulse instance
   * @param T0 = starting time of the pulse
   */
  int TDC(QIEInputPulse* pp, float T0);

  /**
   * Complete set of ADCs for the pulse
   * @param pp = pointer to pulse instance
   */
  std::vector<int> Out_ADC(QIEInputPulse* pp);

  /**
   * Complete set of TDCs for the pulse
   * @param pp = pointer to pulse instance
   */
  std::vector<int> Out_TDC(QIEInputPulse* pp);

  /**
   * Complete set of Capacitor IDs for the pulse
   * @param pp = pointer to pulse instance
   */
  std::vector<int> CapID(QIEInputPulse* pp);

  /**
   * Method to check if the pulse is good to be stored
   * @param pulse pointer to the pulse we want to analyze
   * @note ideally, checks if the amplitude is above some threshold.
   */
  bool PulseCut(QIEInputPulse* pulse, float cut);

 private:
  /// Indices of first bin of each subrange
  int nbins_[5] = {0, 16, 36, 57, 64};
  /// Charge lower limit of all the 16 subranges
  float edges_[17] = {-16,   34,    158,    419,    517,   915,
                      1910,  3990,  4780,   7960,   15900, 32600,
                      38900, 64300, 128000, 261000, 350000};
  /// sensitivity of the subranges (Total charge/no. of bins)
  float sense_[16] = {3.1,   6.2,   12.4,  24.8, 24.8, 49.6, 99.2, 198.4,
                      198.4, 396.8, 793.6, 1587, 1587, 3174, 6349, 12700};

  /// QIE gain -> to convert from no. of e- to charge in fC
  float gain_{1};
  /// time period of one time sample [in ns]
  float tau_{25};
  /// No. of time samples to analyze
  int maxts_{0};

  /// TDC threshold (default 3.74 microAmpere)
  float tdc_thr_{3.74};

  /// Random number generator (required for noise simulation)
  std::unique_ptr<TRandom3> rand_ptr{nullptr};
  TRandom3* trg_;

  /// mean of gaussian noise (Pedestal)
  float mu_{0};
  /// std. dev. of gaussian noise (Actual noise level)
  float sg_{0};
  /// Whether noise is added to the system
  bool isnoise_{false};
};
}  // namespace trigscint
#endif

/**
 * @file Pulse.h
 * @brief class for simulating QIE chip output
 * @author Niramay Gogate, Texas Tech University
 */

#ifndef EVENT_SIMQIE_H
#define EVENT_SIMQIE_H

#include<iostream>
#include"TMath.h"
#include"Event/Pulse.h"
#include"TRandomGen.h"

namespace ldmx {

  /**
   * @class SimQIE
   * @brief class for simulating QIE chip output
   * @note This should be initialized only once per simulation
   */
  class SimQIE
  {
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
     */
    SimQIE(float pd, float sg);

    /**
     * Set gain of SiPM
     * @param gg = gain
     */
    void SetGain(float gg=1e+6);

    /**
     * Set sampling frequency of QIE
     * @param sf = sampling frequency in MHz
     */
    void SetFreq(float sf=40);
    

    /**
     * Quantization error
     */
    float QErr(float Q);

    /**
     * Digitizing input charge
     * @note default charge unit = femto C = 1e-15 C
     */
    int Q2ADC(float);

    /**
     * Converting ADC back to charge
     */
    float ADC2Q(int);


    /**
     * TDC of the input pulse
     * @param pp = pointer to a pulse instance
     * @param T0 = starting time of the pulse
     */
    int TDC(Pulse* pp,float T0);

    /**
     * Complete set of ADCs for the pulse
     * @param pp = pointer to pulse instance
     * @param N = no. of time samples considered for digitization
     */
    int* Out_ADC(Pulse* pp,int N);	// Output per time sample, for N time samples

    /**
     * Complete set of TDCs for the pulse
     * @param pp = pointer to pulse instance
     * @param N = no. of time samples considered for digitization
     */
    int* Out_TDC(Pulse* pp,int N);	// Output per time sample, for N time samples

    /**
     * Complete set of Capacitor IDs for the pulse
     * @param pp = pointer to pulse instance
     * @param N = no. of time samples considered for digitization
     */
    int* CapID(Pulse* pp, int N);	// return CapID for N time samples

 private:

    // Indices of first bin of each subrange
    int nbins[5] = {0,16,36,57,64};
    // Charge lower limit of all the 16 subranges
    float edges[17]={-16, 34, 158, 419, 517, 915, 1910, 3990, 4780, 7960, 15900, 32600, 38900, 64300, 128000, 261000, 350000};
    // sensitivity of the subranges (Total charge/no. of bins)
    float sense[16]={3.1, 6.2, 12.4, 24.8, 24.8, 49.6, 99.2, 198.4, 198.4, 396.8, 793.6, 1587, 1587, 3174, 6349, 12700};

    // QIE gain -> to convert from no. of e- to charge in fC
    float Gain = 1;
    // time period of one time sample [in ns]
    float Tau = 25;

    // TDC threshold (default 3.74 microAmpere)
    float TDC_thr = 3.74;
    // Random number generator (required for noise simulation)
    TRandomGen<ROOT::Math::MixMaxEngine<240,0>>* trg;
    // mean of gaussian noise (Pedestal)
    float mu=0;
    // std. dev. of gaussian noise (Actual noise level)
    float sg=0;
    // Whether noise is added to the system
    bool IsNoise=false;

  };
}
#endif

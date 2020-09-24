// file to store QIE simulation related functions
// default charge unit = femto C = 1e-15 C

#ifndef EVENT_SIMQIE_H
#define EVENT_SIMQIE_H

#include<iostream>
#include"TMath.h"
#include"Event/Pulse.h"
#include"TRandomGen.h"

namespace ldmx {
  class SimQIE
  {
  public:
    SimQIE();		      // without noise & pedestal
    SimQIE(float pd, float sg); // with noise & pedestal

    float QBins[257];		// DO NOT USE. Problem with oevrlapping regions
    void SetGain(float gg=1e+6);	// set gain of QIE
    void SetFreq(float sf=40);	// sampling frequency in MHz
  
    float QErr(float);
    int Q2ADC(float);
    float ADC2Q(int);

    int TDC(Pulse*,float);
    int* Out_ADC(Pulse*,int);	// Output per time sample, for N time samples
    int* Out_TDC(Pulse*,int);	// Output per time sample, for N time samples
    int* CapID(Pulse*, int);	// return CapID for N time samples

  private:
    int nbins[5] = {0,16,36,57,64};
    float edges[17]={-16, 34, 158, 419, 517, 915, 1910, 3990, 4780, 7960, 15900, 32600, 38900, 64300, 128000, 261000, 350000};
    float sense[16]={3.1, 6.2, 12.4, 24.8, 24.8, 49.6, 99.2, 198.4, 198.4, 396.8, 793.6, 1587, 1587, 3174, 6349, 12700};

    float Gain = 1;		// QIE gain -> to convert from no. of e- to charge in fC
    float Tau = 25;		// time period of one time sample [in ns]

    float TDC_thr = 3.74;				    // TDC threshold - 3.74 microAmpere
    TRandomGen<ROOT::Math::MixMaxEngine<240,0>>* trg; // Random number generator
    float mu=0;			                    // mean of gaussian noise (Pedestal)
    float sg=0;			                    // std. dev. of gaussian noise (Actual noise level)
    bool IsNoise=false;		                    // Whether noise is added to the system

    void GenerateBins();		                    // DO NOT USE. Problem with overalapping regions
  };
}
#endif

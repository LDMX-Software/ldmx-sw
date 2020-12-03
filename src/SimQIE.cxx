/**
 * @file SimQIE.cxx
 * @author Niramay Gogate, Texas Tech University
 */

#include<iostream>
#include <exception>
#include"TMath.h"
#include"TrigScint/SimQIE.h"
#include "Framework/Exception.h"
namespace ldmx {

  SimQIE::SimQIE() {}

  SimQIE::SimQIE(float PD, float SG, uint64_t seed=0) {
    isnoise = true;
    if (seed==0) {
      EXCEPTION_RAISE("RandomSeedException",
		      "QIE Noise generator not seeded (seed=0)");
    }
    else{
      rand_ptr = std::make_unique<TRandom3>(seed);
      trg = rand_ptr.get();
    }
    mu = PD;
    sg = SG;
  }

  void SimQIE::SetGain(float gg) {
    gain = gg*16e-5;		// to convert from 1.6e-19 to fC
  }

  void SimQIE::SetFreq(float sf) {
    tau = 1000/sf;		// 1/sf -> MHz to ns
  }

  void SimQIE::SetNTimeSamples(int maxts) {
    maxts_ = maxts;
  }
  // Function to convert charge to ADC count
  // Working: The method checks in which QIE subrange does the charge lie,
  // applies a corresponding  gain to it and digitizes it.
  int SimQIE::Q2ADC(float QQ) {
    float qq = gain*QQ;		    // including QIE gain
    if (isnoise) qq+=trg->Gaus(mu,sg); // Adding gaussian random noise.

    if (qq<=edges[0]) return 0;
    if (qq>=edges[16]) return 255;

    int ID=8;
    int a=0;
    int b=16;
    while(b-a!=1) {
      if (qq>edges[(a+b)/2]) {
	a=(a+b)/2;
      }
      else b=(a+b)/2;
    }
    return 64*(int)(a/4)+nbins[a%4]+floor((qq-edges[a])/sense[a]);
  }

  // Function to convert ADCs back to charge
  // Working: The method checks to which QIE subrange does the ADC correspnd to
  // and returns the mean charge of the correspnding bin in the subrange
  float SimQIE::ADC2Q(int adc) {
    if (adc<= 0) return -16;
    if (adc>= 255) return 350000;

    int rr = adc/64;		// range
    int v1 = adc%64;		// temp. var
    int ss = 0;			// sub range
  
    for(int i=1;i<4;i++) {		// to get the subrange
      if (v1>nbins[i]) ss++;
    }
    int cc = 64*rr+nbins[ss];
    float temp =
      edges[4*rr+ss]+
      (v1-nbins[ss])*sense[4*rr+ss]+
      sense[4*rr+ss]/2;
    return(temp/gain);
  }

  // Function to return the quantization error for given input charge
  float SimQIE::QErr(float Q) {
    if (Q<=edges[0]) return 0;
    if (Q>=edges[16]) return 0;

    int ID=8;
    int a=0;
    int b=16;
    while(b-a!=1) {
      if (Q>edges[(a+b)/2]) a=(a+b)/2;
      else b=(a+b)/2;
    }
    return(sense[a]/(sqrt(12)*Q));
  }

  // Function that returns an array of ADCs each corresponding to
  // one time sample
  int* SimQIE::Out_ADC(QIEInputPulse* pp) {
    int* OP = new int[maxts_];

    for(int i=0;i<maxts_;i++) {
      float QQ = pp->Integrate(i*tau,i*tau+tau);
      OP[i]=Q2ADC(QQ);
    }
    return OP;
  }

  // Function that returns the digitized time corresponding to
  // current pulse crossing a specified current threshold
  int SimQIE::TDC(QIEInputPulse* pp, float T0=0) {
    float thr2=tdc_thr/gain;
    if (pp->Eval(T0)>thr2) return 62;		// when pulse starts high
    for(float tt=T0;tt<T0+tau;tt+=0.1) {
      if (pp->Eval(tt)>=thr2) return((int)(2*(tt-T0)));
    }
    return 63;			// when pulse remains low all along
  }

  // Function that returns an array of TDCs each corresponding to
  // one time sample
  int* SimQIE::Out_TDC(QIEInputPulse* pp) {
    int* OP = new int[maxts_];

    for(int i=0;i<maxts_;i++) {
      OP[i] = TDC(pp,tau*i);
    }
    return OP;
  }

  // Function that returns an array of Caoacitor IDs
  // each corresponding to one time sample
  int* SimQIE::CapID(QIEInputPulse* pp) {
    int* OP = new int[maxts_];
    OP[0] = trg->Integer(4);
    for(int i=0;i<maxts_;i++) {
      OP[i+1]=(OP[i]+1)%4;
    }
    return OP;
  }
  
  bool SimQIE::PulseCut(QIEInputPulse* pulse) {
    if(pulse->npulses==0) return false;

    // Only keep the pulse if it produces 1 PE
    // in atleast one of the time samples
    float thr_in_pes=1.0;

    for(int i=0;i<maxts_;i++){
      if(pulse->Integrate(i*tau,i*tau+tau)>=thr_in_pes) return true;
    }
    return false;
  }
}

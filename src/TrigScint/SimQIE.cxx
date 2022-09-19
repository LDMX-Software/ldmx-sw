#include "TrigScint/SimQIE.h"
#include <exception>
#include <iostream>
#include "Framework/Exception/Exception.h"
#include "TMath.h"

namespace trigscint {

SimQIE::SimQIE() {}

SimQIE::SimQIE(float PD, float SG, uint64_t seed = 0) {
  isnoise_ = true;
  if (seed == 0) {
    EXCEPTION_RAISE("RandomSeedException",
                    "QIE Noise generator not seeded (seed=0)");
  } else {
    rand_ptr = std::make_unique<TRandom3>(seed);
    trg_ = rand_ptr.get();
  }
  mu_ = PD;
  sg_ = SG;
}

// Function to convert charge to ADC count
// Working: The method checks in which QIE subrange does the charge lie,
// applies a corresponding  gain to it and digitizes it.
int SimQIE::Q2ADC(float Charge) {
  float qq = gain_ * Charge;                 // including QIE gain
  if (isnoise_) qq += trg_->Gaus(mu_, sg_);  // Adding gaussian random noise.

  if (qq <= edges_[0]) return 0;
  if (qq >= edges_[16]) return 255;

  int ID = 8;
  int a = 0;
  int b = 16;

  // Binary search to find the subrange
  while (b - a != 1) {
    if (qq > edges_[(a + b) / 2]) {
      a = (a + b) / 2;
    } else
      b = (a + b) / 2;
  }
  return 64 * (int)(a / 4) + nbins_[a % 4] +
         floor((qq - edges_[a]) / sense_[a]);
}

// Function to convert ADCs back to charge
// The method checks to which QIE subrange does the ADC correspnd to
// and returns the mean charge of the correspnding bin in the subrange
float SimQIE::ADC2Q(int ADC) {
  if (ADC <= 0) return -16;
  if (ADC >= 255) return 350000;

  int rr = ADC / 64;  // range
  int v1 = ADC % 64;  // temp. var
  int ss = 0;         // sub range

  for (int i = 1; i < 4; i++) {  // to get the subrange
    if (v1 > nbins_[i]) ss++;
  }
  int cc = 64 * rr + nbins_[ss];
  float temp = edges_[4 * rr + ss] + (v1 - nbins_[ss]) * sense_[4 * rr + ss] +
               sense_[4 * rr + ss] / 2;
  return (temp / gain_);
}

// Function to return the quantization error for given input charge
float SimQIE::QErr(float Q) {
  if (Q <= edges_[0]) return 0;
  if (Q >= edges_[16]) return 0;

  int ID = 8;
  int a = 0;
  int b = 16;
  while (b - a != 1) {
    if (Q > edges_[(a + b) / 2])
      a = (a + b) / 2;
    else
      b = (a + b) / 2;
  }
  return (sense_[a] / (sqrt(12) * Q));
}

// Function that returns an array of ADCs each corresponding to
// one time sample
std::vector<int> SimQIE::Out_ADC(QIEInputPulse* pp) {
  std::vector<int> OP;

  for (int i = 0; i < maxts_; i++) {
    float QQ = pp->Integrate(i * tau_, i * tau_ + tau_);
    OP.push_back(Q2ADC(QQ));
  }
  return OP;
}

// Function that returns the digitized time corresponding to
// current pulse crossing a specified current threshold
int SimQIE::TDC(QIEInputPulse* pp, float T0 = 0) {
  float thr2 = tdc_thr_ / gain_;
  if (pp->Eval(T0) > thr2) return 62;  // when pulse starts high
  for (float tt = T0; tt < T0 + tau_; tt += 0.1) {
    if (pp->Eval(tt) >= thr2) return ((int)(2 * (tt - T0)));
  }
  return 63;  // when pulse remains low all along
}

// Function that returns an array of TDCs each corresponding to
// one time sample
std::vector<int> SimQIE::Out_TDC(QIEInputPulse* pp) {
  std::vector<int> OP;

  for (int i = 0; i < maxts_; i++) {
    OP.push_back(TDC(pp, tau_ * i));
  }
  return OP;
}

// Function that returns an array of Caoacitor IDs
// each corresponding to one time sample
std::vector<int> SimQIE::CapID(QIEInputPulse* pp) {
  std::vector<int> OP;

  OP.push_back(trg_->Integer(4));
  for (int i = 0; i < maxts_; i++) {
    OP.push_back((OP[i] + 1) % 4);
  }
  return OP;
}

  bool SimQIE::PulseCut(QIEInputPulse* pulse, float cut) {
  if (pulse->GetNPulses() == 0) return false;

  //  float thr_in_pes = 1.0; instead make configurable 

  // Only keep the pulse if it produces 1 PE (or whatever the cutoff is set to)
  //integrate over entire pulse so we catch also single-PE pulses 
  float integral=0;
  for (int i = 0; i < maxts_; i++) {
	//    if (pulse->Integrate(i * tau_, i * tau_ + tau_) >= thr_in_pes) return true;
	integral+= pulse->Integrate(i * tau_, i * tau_ + tau_);
  }
  if ( integral >= cut) return true;
  
  return false;
}
}  // namespace trigscint

/**
 * @file QIEInputPulse.cxx
 * @author Niramay Gogate, Texas Tech University
 */

#include<iostream>
#include "TrigScint/QIEInputPulse.h"
#include<cmath>

namespace trigscint {

  void QIEInputPulse::AddPulse(float toff, float ampl){
    toff_.push_back(toff);
    ampl_.push_back(ampl);
    npulses++;
  }

  float QIEInputPulse::Eval(float T){
    if(npulses==0) return 0;
    float val=0;
    for(int i=0;i<npulses;i++){
      val+=EvalSingle(T,i);
    }
    return val;
  }
  
  // Bimoid pulse, made out of difference of two sigmoids parametrized by
  // rt,ft respectively.
  // Parameters:
  // rise = rise time
  // fall = fall time
  Bimoid::Bimoid(float rise,float fall){
    rt = rise;
    ft = fall;
  }

  float Bimoid::EvalSingle(float T, int id){
    if (T<toff_[id]) return 0;
    // Normalization constant
    float nc = (ft-rt)*log(2)/ampl_[id];
    
    float y1 = 1/(1+exp((toff_[id]-T)/rt));
    float y2 = 1/(1+exp((toff_[id]-T)/ft));
    return((y1-y2)/nc);
  }

  float Bimoid::Integrate(float T1, float T2){
    float val=0;
    for(int id=0;id<npulses;id++){
      if (ampl_[id]>0 && T2>toff_[id]){
	val += I_Int(T2,id)-I_Int(T1,id);
      }
    }
    return val;
  }

  float Bimoid::I_Int(float T, int id){
    if (T<=toff_[id]) return 0;
    // Normalization constant
    float nc = (ft-rt)*log(2)/ampl_[id];

    float t=T-toff_[id];	// time relative to offset

    float II =			// Integral
      rt*log(1+exp((t-toff_[id])/rt)) -
      ft*log(1+exp((t-toff_[id])/ft));

    return II/nc;
  }


  float Bimoid::Max(int id){
    float a = 0;
    float b = 50;
    float mx=(a+b)/2;		// maximum

    while(abs(Derivative(mx,id))>=1e-5){
      if (Derivative(a,id)*Derivative(mx,id)>0) {
    	a=mx;
      }
      else b = mx;
      mx = (a+b)/2;
    }
    return(mx);
  }

  float Bimoid::Derivative(float T, int id){
    // Normalization constant
    float nc = (ft-rt)*log(2)/ampl_[id];

    float T_ = T-toff_[id];
    float E1 = exp(-T_/rt);
    float E2 = exp(-T_/ft);

    float v1 = E1/(rt*pow(1+E1,2));
    float v2 = E2/(ft*pow(1+E2,2));

    return((v1-v2)/nc);		// Actual derivative
  }

  Expo::Expo(){}

  // A current pulse formed by assuming SiPM as an ideal capacitor which is
  // fed with a constant current.
  // Parameters:
  // k_ = 1/(RC time constant of the capacitor)
  // tmax_ = The charging time of the capacitor
  Expo::Expo(float k_,float tmax_){
    k=k_;
    tmax=tmax_;

    rt = (log(9+exp(-k*tmax))-log(1+9*exp(-k*tmax)))/k;
    ft = log(9)/k;
  }

  // Manually set the rise time and fall time of the pulse
  void Expo::SetRiseFall(float rr, float ff){
    rt=rr;
    ft=ff;
  
    k = log(9)/ft;
    tmax = (log(9-exp(-k*rt))-log(9*exp(-k*rt)-1))/k;
  }

  float Expo::EvalSingle(float t_,int id){
    if (id>=npulses) return 0;
    if (t_<=toff_[id]) return 0;
    if (ampl_[id]==0) return 0;

    //Normalization constant
    float nc = ampl_[id]/tmax;
    // time relative to the offset
    float T = t_-toff_[id];
    if (T<tmax) {
      return(nc*(1-exp(-k*T)));
    }
    else {
      return(nc*(1-exp(-k*tmax))*exp(k*(tmax-T)));
    }
    return -1;
  }

  float Expo::Max(int id){
    //Normalization constant
    float nc = ampl_[id]/tmax;
    return nc*(1-exp(-k*tmax));
  }

  float Expo::Integrate(float T1, float T2){
    float val=0;
    for(int id=0;id<npulses;id++){
      if (ampl_[id]>0 && T2>toff_[id]){
	val += I_Int(T2,id)-I_Int(T1,id);
      }
    }
    return val;
  }

float Expo::Derivative(float T, int id){
    if (id>=npulses) return 0;
    if (T<=toff_[id]) return 0;

    float t=T-toff_[id];
    //Normalization constant
    float nc = ampl_[id]/tmax;

    if (t<=tmax) return(nc*k*exp(-k*t));
    return(-nc*k*(1-exp(-k*tmax))*exp(k*(tmax-t)));
  }

float Expo::I_Int(float T, int id){
    if (T<=toff_[id]) return 0;
    float t=T-toff_[id];
    //Normalization constant
    float nc = ampl_[id]/tmax;
    if (t<tmax) return(nc*(k*t+exp(-k*t)-1)/k);

    float c1 = (1-exp(-k*tmax))/k;
    float c2 = tmax-c1*exp(k*(tmax-t));
    return nc*c2;
  }

}

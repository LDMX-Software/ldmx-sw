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
  }

  float QIEInputPulse::Eval(float T){
    if(ampl_.size()==0) return 0;
    float val=0;
    for(int i=0;i<ampl_.size();i++){
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
    rt_ = rise;
    ft_ = fall;
  }

  float Bimoid::EvalSingle(float T, int id){
    if (T<toff_[id]) return 0;
    // Normalization constant
    float nc = (ft_-rt_)*log(2)/ampl_[id];
    
    float y1 = 1/(1+exp((toff_[id]-T)/rt_));
    float y2 = 1/(1+exp((toff_[id]-T)/ft_));
    return((y1-y2)/nc);
  }

  float Bimoid::Integrate(float T1, float T2){
    float val=0;
    for(int id=0;id<ampl_.size();id++){
      if (ampl_[id]>0 && T2>toff_[id]){
	val += I_Int(T2,id)-I_Int(T1,id);
      }
    }
    return val;
  }

  float Bimoid::I_Int(float T, int id){
    if (T<=toff_[id]) return 0;
    // Normalization constant
    float nc = (ft_-rt_)*log(2)/ampl_[id];

    float t=T-toff_[id];	// time relative to offset

    float II =			// Integral
      rt_*log(1+exp((t-toff_[id])/rt_)) -
      ft_*log(1+exp((t-toff_[id])/ft_));

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
    float nc = (ft_-rt_)*log(2)/ampl_[id];

    float T_ = T-toff_[id];
    float E1 = exp(-T_/rt_);
    float E2 = exp(-T_/ft_);

    float v1 = E1/(rt_*pow(1+E1,2));
    float v2 = E2/(ft_*pow(1+E2,2));

    return((v1-v2)/nc);		// Actual derivative
  }

  Expo::Expo(){}

  // A current pulse formed by assuming SiPM as an ideal capacitor which is
  // fed with a constant current.
  // Parameters:
  // k_ = 1/(RC time constant of the capacitor)
  // tmax_ = The charging time of the capacitor
  Expo::Expo(float k,float tmax){
    k_=k;
    tmax_=tmax;

    rt_ = (log(9+exp(-k_*tmax_))-log(1+9*exp(-k_*tmax_)))/k_;
    ft_ = log(9)/k_;
  }

  // Manually set the rise time and fall time of the pulse
  void Expo::SetRiseFall(float rr, float ff){
    rt_=rr;
    ft_=ff;
  
    k_ = log(9)/ft_;
    tmax_ = (log(9-exp(-k_*rt_))-log(9*exp(-k_*rt_)-1))/k_;
  }

  float Expo::EvalSingle(float t_,int id){
    if (id>=ampl_.size()) return 0;
    if (t_<=toff_[id]) return 0;
    if (ampl_[id]==0) return 0;

    //Normalization constant
    float nc = ampl_[id]/tmax_;
    // time relative to the offset
    float T = t_-toff_[id];
    if (T<tmax_) {
      return(nc*(1-exp(-k_*T)));
    }
    else {
      return(nc*(1-exp(-k_*tmax_))*exp(k_*(tmax_-T)));
    }
    return -1;
  }

  float Expo::Max(int id){
    //Normalization constant
    float nc = ampl_[id]/tmax_;
    return nc*(1-exp(-k_*tmax_));
  }

  float Expo::Integrate(float T1, float T2){
    float val=0;
    for(int id=0;id<ampl_.size();id++){
      if (ampl_[id]>0 && T2>toff_[id]){
	val += I_Int(T2,id)-I_Int(T1,id);
      }
    }
    return val;
  }

float Expo::Derivative(float T, int id){
    if (id>=ampl_.size()) return 0;
    if (T<=toff_[id]) return 0;

    float t=T-toff_[id];
    //Normalization constant
    float nc = ampl_[id]/tmax_;

    if (t<=tmax_) return(nc*k_*exp(-k_*t));
    return(-nc*k_*(1-exp(-k_*tmax_))*exp(k_*(tmax_-t)));
  }

float Expo::I_Int(float T, int id){
    if (T<=toff_[id]) return 0;
    float t=T-toff_[id];
    //Normalization constant
    float nc = ampl_[id]/tmax_;
    if (t<tmax_) return(nc*(k_*t+exp(-k_*t)-1)/k_);

    float c1 = (1-exp(-k_*tmax_))/k_;
    float c2 = tmax_-c1*exp(k_*(tmax_-t));
    return nc*c2;
  }

}

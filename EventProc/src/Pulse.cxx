// // Class to store all the functions related to the input pulse to QIE
// #include "EventProc/Pulse.h"
// #include<cmath>
// namespace ldmx {

//   Bimoid::Bimoid(float start,float qq=1)
//     {
//       t0 = start;
//       rt = 1;			// default rise and fall tiems
//       ft = 10;
//       Q0 = qq;
//       NC = (ft-rt)*log(2)/Q0;		// Remember, u hv to divide by it
//     }

//   Bimoid::Bimoid(float start,float rise, float fall, float qq=1)
//     {
//       t0 = start;
//       rt = rise;
//       ft = fall;
//       Q0 = qq;
//       NC = (ft-rt)*log(2)/Q0;		// Remember, u hv to divide by it
//     }

//   float Bimoid::eval(float T)
//   {
//     if(T<t0) return(0);
//     float y1 = 1/(1+exp((t0-T)/rt));
//     float y2 = 1/(1+exp((t0-T)/ft));
//     return((y1-y2)/NC);
//   }

//   float Bimoid::Integrate(float T1, float T2)
//   {
//     if(T2<t0) return(0);
//     float t1 = T1;
//     float t2 = T2;
//     if(T1<0) t1 = 0;
  
//     float I1 = rt*log(1+exp((t1-t0)/rt)) - ft*log(1+exp((t1-t0)/ft));
//     float I2 = rt*log(1+exp((t2-t0)/rt)) - ft*log(1+exp((t2-t0)/ft));
//     return((I2-I1)/NC);
//   }

//   float Bimoid::Max()
//   {
//     float a = t0;
//     float b =t0+10;
//     float mx=(a+b)/2;		// maximum

//     while(abs(Der(mx))>=1e-5){
//       if(Der(a)*Der(mx)>0) a=mx;
//       else b = mx;
//       mx = (a+b)/2;
//     }
//     return(mx);
//   }

//   float Bimoid::Der(float T)
//   {
//     float T_ = T-t0;
//     float E1 = exp(-T_/rt);
//     float E2 = exp(-T_/ft);

//     float v1 = E1/(rt*pow(1+E1,2));
//     float v2 = E2/(ft*pow(1+E2,2));

//     return((v1-v2)/NC);		// Actual derivative
//   }

//   Expo::Expo(){}
//   Expo::Expo(float k_,float tmax_,float tstart_=0,float Q_=1)
//     {
//       k=k_;
//       tmax=tmax_;
//       t0=tstart_;
//       NC = Q_/tmax;

//       rt = (log(9+exp(-k*tmax))-log(1+9*exp(-k*tmax)))/k;
//       ft = log(9)/k;
//     }

//   void Expo::SetRiseFall(float rr, float ff)
//   {
//     rt=rr;
//     ft=ff;
  
//     k = log(9)/ft;
//     tmax = (log(9-exp(-k*rt))-log(9*exp(-k*rt)-1))/k;
//   }

//   float Expo::eval(float t_)
//   {
//     if(NC==0) return(0);		// fast evaluation for zero pulse
//     if(t_<=t0) return(0);
//     float T = t_-t0;
//     if(T<tmax) return(NC*(1-exp(-k*T))/tmax);
//     else{
//       return(NC*(1-exp(-k*tmax))*exp(k*(tmax-T))/tmax);
//     }
//     return(-1);
//   }

//   float Expo::Max()
//   {
//     return NC*(1-exp(-k*tmax))/tmax;
//   }

//   float Expo::Integrate(float T1, float T2)
//   {
//     if(NC==0) return(0);		// for faster implementation of zero pulse
//     if(T2<=t0) return(0);
//     return(I_Int(T2)-I_Int(T1));
//   }

//   float Expo::Der(float T)
//   {
//     if(T<=t0) return(0);
//     float t=T-t0;
//     if(t<=tmax) return(NC*k*exp(-k*t));
//     return(-NC*k*(1-exp(-k*tmax))*exp(k*(tmax-t)));
//   }

//   float Expo::I_Int(float T)
//   {
//     if(T<=t0) return(0);
//     float t=T-t0;
//     if(t<tmax) return(NC*(k*t+exp(-k*t)-1)/k);

//     float c1 = (1-exp(-k*tmax))/k;
//     float c2 = tmax-c1*exp(k*(tmax-t));
//     return(NC*c2);
//   }
// }

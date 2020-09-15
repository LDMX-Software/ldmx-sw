// Class to store all the functions related to the input pulse to QIE

////////////////////////// The base class ///////////////////////////////
class Pulse
{
public:
  virtual float eval(float T);		      // Evaluate pulse at time T
  virtual float Integrate(float T1,float T2); // Integrate pulse from T1 to T2
  virtual float Der(float T);		      // Differentiate pulse at time T
  virtual float Max();			      // Return the maximum of the pulse
};

float Pulse::eval(float T){return(0);}
float Pulse::Integrate(float T1,float T2){return(0);}
float Pulse::Der(float T){return(0);}
float Pulse::Max(){return(0);}
//////////////////////// Daughter class /////////////////////////////////

class Bimoid: public Pulse
{
public:
  Bimoid(float start,float rise, float fall, float qq=1);
  Bimoid(float start,float qq=1);
  float eval(float T);
  float Integrate(float T1,float T2);
  float Max();
  float Der(float T);
private:
  float t0;			// starting time of the pulse
  float rt;			// rise time
  float ft;			// fall time
  float NC;			// Normalization constant
  float Q0;			// Net charge (integral I.dt)
};

////////////////piece-wise Exponential pulse/////////////////////////////

class Expo: public Pulse
{
public:
  Expo();					   // default constructor
  Expo(float k_,float tmax_,float tstart_=0,float Q_=1); // main constructor

  float GetRise(){return(rt);}	// return rise time
  float GetFall(){return(ft);}	// return fall time
  void SetRiseFall(float rr,float ff);

  float eval(float T);
  float Integrate(float T1,float T2);
  float Max();
  float Der(float T);
private:
  float t0;			// pulse start time
  float k;			// 1/RC time constant (for the capacitor)
  float tmax;			// time when pulse attains maximum
  float NC;			// normalization constant
  float rt=-1;			// rise time
  float ft=-1;			// fall time

  float I_Int(float T);		// Indefinite integral
};
////////////////piece-wise Exponential pulse/////////////////////////////

Bimoid::Bimoid(float start,float qq=1)
{
  t0 = start;
  rt = 1;			// default rise and fall tiems
  ft = 10;
  Q0 = qq;
  NC = (ft-rt)*log(2)/Q0;		// Remember, u hv to divide by it
}

Bimoid::Bimoid(float start,float rise, float fall, float qq=1)
{
  t0 = start;
  rt = rise;
  ft = fall;
  Q0 = qq;
  NC = (ft-rt)*log(2)/Q0;		// Remember, u hv to divide by it
}

float Bimoid::eval(float T)
{
  if(T<t0) return(0);
  float y1 = 1/(1+exp((t0-T)/rt));
  float y2 = 1/(1+exp((t0-T)/ft));
  return((y1-y2)/NC);
}

float Bimoid::Integrate(float T1, float T2)
{
  if(T2<t0) return(0);
  float t1 = T1;
  float t2 = T2;
  if(T1<0) t1 = 0;
  
  float I1 = rt*log(1+exp((t1-t0)/rt)) - ft*log(1+exp((t1-t0)/ft));
  float I2 = rt*log(1+exp((t2-t0)/rt)) - ft*log(1+exp((t2-t0)/ft));
  return((I2-I1)/NC);
}

float Bimoid::Max()
{
  float a = t0;
  float b =t0+10;
  float mx=(a+b)/2;		// maximum

  while(abs(Der(mx))>=1e-5){
    if(Der(a)*Der(mx)>0) a=mx;
    else b = mx;
    mx = (a+b)/2;
  }
  return(mx);
}

float Bimoid::Der(float T)
{
  float T_ = T-t0;
  float E1 = exp(-T_/rt);
  float E2 = exp(-T_/ft);

  float v1 = E1/(rt*pow(1+E1,2));
  float v2 = E2/(ft*pow(1+E2,2));

  return((v1-v2)/NC);		// Actual derivative
}

Expo::Expo(){}
Expo::Expo(float k_,float tmax_,float tstart_=0,float Q_=1)
{
  k=k_;
  tmax=tmax_;
  t0=tstart_;
  NC = Q_/tmax;

  rt = (log(9+exp(-k*tmax))-log(1+9*exp(-k*tmax)))/k;
  ft = log(9)/k;
}

void Expo::SetRiseFall(float rr, float ff)
{
  rt=rr;
  ft=ff;
  
  k = log(9)/ft;
  tmax = (log(9-exp(-k*rt))-log(9*exp(-k*rt)-1))/k;
}

float Expo::eval(float t_)
{
  if(NC==0) return(0);		// fast evaluation for zero pulse
  if(t_<=t0) return(0);
  float T = t_-t0;
  if(T<tmax) return(NC*(1-exp(-k*T))/tmax);
  else{
    return(NC*(1-exp(-k*tmax))*exp(k*(tmax-T))/tmax);
  }
  return(-1);
}

float Expo::Max()
{
  return NC*(1-exp(-k*tmax))/tmax;
}

float Expo::Integrate(float T1, float T2)
{
  if(NC==0) return(0);		// for faster implementation of zero pulse
  if(T2<=t0) return(0);
  return(I_Int(T2)-I_Int(T1));
}

float Expo::Der(float T)
{
  if(T<=t0) return(0);
  float t=T-t0;
  if(t<=tmax) return(NC*k*exp(-k*t));
  return(-NC*k*(1-exp(-k*tmax))*exp(k*(tmax-t)));
}

float Expo::I_Int(float T)
{
  if(T<=t0) return(0);
  float t=T-t0;
  if(t<tmax) return(NC*(k*t+exp(-k*t)-1)/k);

  float c1 = (1-exp(-k*tmax))/k;
  float c2 = tmax-c1*exp(k*(tmax-t));
  return(NC*c2);
}

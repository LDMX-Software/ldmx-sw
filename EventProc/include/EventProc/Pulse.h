// Class to store all the functions related to the input pulse to QIE

////////////////////////// The base class ///////////////////////////////
namespace ldmx {
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
    /* Bimoid(float start,float rise, float fall, float qq=1); */
    /* Bimoid(float start,float qq=1); */
    Bimoid(float start,float rise, float fall, float qq);
    Bimoid(float start,float qq);
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
    /* Expo(float k_,float tmax_,float tstart_=0,float Q_=1); // main constructor */
    Expo(float k_,float tmax_,float tstart_,float Q_); // main constructor

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

}

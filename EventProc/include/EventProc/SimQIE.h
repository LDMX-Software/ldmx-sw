// file to store QIE simulation related functions
// default charge unit = femto C = 1e-15 C
#include"TMath.h"
#include"Pulse.h"

class SimQIE
{
public:
  SimQIE(bool DBG=false);		      // without noise & pedestal
  SimQIE(float pd, float sg, bool DBG=false); // with noise & pedestal

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
  int bins[5] = {0,16,36,57,64};
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

SimQIE::SimQIE(bool DBG=false){}	// for debugging turn it on

SimQIE::SimQIE(float PD, float SG, bool DBG=false)
{
  IsNoise = true;
  trg = new TRandomGen<ROOT::Math::MixMaxEngine<240,0>>();
  mu = PD;
  sg = SG;
}

void SimQIE::SetGain(float gg=1e+6)
{
  Gain = gg*16e-5;		// to convert from 1.6e-19 to fC
}

void SimQIE::SetFreq(float sf = 40)
{
  Tau = 1000/sf;		// 1/sf -> MHz to ns
}

int SimQIE::Q2ADC(float QQ)
{
  float qq = Gain*QQ;		    // including QIE gain
  if(IsNoise) qq+=trg->Gaus(mu,sg); // Adding gaussian random noise.

  if(qq<=edges[0]) return(0);
  if(qq>=edges[16]) return(255);

  int ID=8;
  int a=0;
  int b=16;
  while(b-a!=1){
    if(qq>edges[(a+b)/2]) a=(a+b)/2;
    else b=(a+b)/2;
  }
  return 64*(int)(a/4)+bins[a%4]+floor((qq-edges[a])/sense[a]);
}

float SimQIE::ADC2Q(int adc)
{
  if(adc<= 0) return(-16);
  if(adc>= 255) return(350000);

  int rr = adc/64;		// range
  int v1 = adc%64;		// temp. var
  int ss = 0;			// sub range
  
  for(int i=1;i<4;i++){		// to get the subrange
    if(v1>bins[i]) ss++;
  }
  int cc = 64*rr+bins[ss];
  // return(edges[4*rr+ss]+(v1-bins[ss])*sense[4*rr+ss]+sense[4*rr+ss]/2);
  float temp=edges[4*rr+ss]+(v1-bins[ss])*sense[4*rr+ss]+sense[4*rr+ss]/2;
  return(temp/Gain);
}

float SimQIE::QErr(float Q)
{
  if(Q<=edges[0]) return(0);
  if(Q>=edges[16]) return(0);

  int ID=8;
  int a=0;
  int b=16;
  while(b-a!=1){
    if(Q>edges[(a+b)/2]) a=(a+b)/2;
    else b=(a+b)/2;
  }
  return(sense[a]/(sqrt(12)*Q));
}

void SimQIE::GenerateBins()
{
  float charge;
  QBins[0]=edges[0];

  for(int rr=0;rr<4;rr++){	         // loop over range
    for(int ss=0;ss<4;ss++){	         // loop over subrange
      for(int bb=bins[ss]; bb<bins[ss+1];bb++){ // loop over bins
	cout<<"rr = "<<rr<<"\tss = "<<ss<<"\tbb = "<<bb<<endl;
	int ind = 64*rr+bb;
	QBins[ind]=edges[4*rr+ss]+(bb-bins[ss])*sense[4*rr+ss];
      }
    }
  }
  
}

int* SimQIE::Out_ADC(Pulse* pp,int N)
{
  int* OP = new int[N];	// N no. of output ADCs

  for(int i=0;i<N;i++){
    float QQ = pp->Integrate(i*Tau,i*Tau+Tau);
    OP[i]=Q2ADC(QQ);
  }
  return(OP);
}

int SimQIE::TDC(Pulse* pp, float T0=0)
{
  float thr2=TDC_thr/Gain;
  if(pp->eval(T0)>thr2) return(62);		// when pulse starts high
  for(float tt=T0;tt<T0+Tau;tt+=0.1){
    if(pp->eval(tt)>=thr2) return((int)(2*(tt-T0)));
  }
  return(63);			// when pulse remains low all along
}

int* SimQIE::Out_TDC(Pulse* pp,int N)
{
  int* OP = new int[N];	// N no. of output ADCs

  for(int i=0;i<N;i++){
    OP[i] = TDC(pp,Tau*i);
  }
  return(OP);
}

int* SimQIE::CapID(Pulse* pp, int N)
{
  int* OP = new int[N+1];	// N no. of output CapIDs
  OP[0]=0;			// needs to be changed later
  TRandomGen<ROOT::Math::MixMaxEngine<240,0>> rng;
  OP[1]=rng.Integer(4);
  for(int i=1;i<N;i++){
    OP[i+1]=(OP[i]+1)%4;
  }
  return(OP);

}

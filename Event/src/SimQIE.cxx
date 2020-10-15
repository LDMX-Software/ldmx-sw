/**
 * @file SimQIE.cxx
 * @author Niramay Gogate, Texas Tech University
 */

#include<iostream>
#include <exception>
#include"TMath.h"
#include"Event/SimQIE.h"

namespace ldmx {

SimQIE::SimQIE(){}

SimQIE::SimQIE(float PD, float SG)
{
  IsNoise = true;
  trg = new TRandomGen<ROOT::Math::MixMaxEngine<240,0>>();
  mu = PD;
  sg = SG;
}

void SimQIE::SetGain(float gg)
{
  Gain = gg*16e-5;		// to convert from 1.6e-19 to fC
}

void SimQIE::SetFreq(float sf)
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
  return 64*(int)(a/4)+nbins[a%4]+floor((qq-edges[a])/sense[a]);
}

float SimQIE::ADC2Q(int adc)
{
  if(adc<= 0) return(-16);
  if(adc>= 255) return(350000);

  int rr = adc/64;		// range
  int v1 = adc%64;		// temp. var
  int ss = 0;			// sub range
  
  for(int i=1;i<4;i++){		// to get the subrange
    if(v1>nbins[i]) ss++;
  }
  int cc = 64*rr+nbins[ss];
  // return(edges[4*rr+ss]+(v1-nbins[ss])*sense[4*rr+ss]+sense[4*rr+ss]/2);
  float temp=edges[4*rr+ss]+(v1-nbins[ss])*sense[4*rr+ss]+sense[4*rr+ss]/2;
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

int* SimQIE::Out_ADC(QIEInputPulse* pp,int N)
{
  int* OP = new int[N];	// N no. of output ADCs

  for(int i=0;i<N;i++){
    float QQ = pp->Integrate(i*Tau,i*Tau+Tau);
    OP[i]=Q2ADC(QQ);
  }
  return(OP);
}

int SimQIE::TDC(QIEInputPulse* pp, float T0=0)
{
  float thr2=TDC_thr/Gain;
  if(pp->eval(T0)>thr2) return(62);		// when pulse starts high
  for(float tt=T0;tt<T0+Tau;tt+=0.1){
    if(pp->eval(tt)>=thr2) return((int)(2*(tt-T0)));
  }
  return(63);			// when pulse remains low all along
}

int* SimQIE::Out_TDC(QIEInputPulse* pp,int N)
{
  int* OP = new int[N];	// N no. of output ADCs

  for(int i=0;i<N;i++){
    OP[i] = TDC(pp,Tau*i);
  }
  return(OP);
}

int* SimQIE::CapID(QIEInputPulse* pp, int N)
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
}

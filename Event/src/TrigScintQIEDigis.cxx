#include "Event/TrigScintQIEDigis.h"
#include <iostream>
#include <exception>
ClassImp(ldmx::TrigScintQIEDigis);

namespace ldmx {
  
  TrigScintQIEDigis::TrigScintQIEDigis()
  {
  }

  // TrigScintQIEDigis::~TrigScintQIEDigis(){}

  TrigScintQIEDigis::TrigScintQIEDigis(int maxTS_,Pulse* pl, float pd=0, float ns=0)
  {
    SimQIE* smq;
    if(ns==0) smq = new SimQIE();
    else smq = new SimQIE(pd,ns);
    smq->SetGain();		// by default, 1e+6
    smq->SetFreq();		// by default, 40
    maxTS = maxTS_;
    // ADCs = smq->Out_ADC(pl,maxTS);
    // TDCs = smq->Out_TDC(pl,maxTS);
    // CIDs = smq->CapID(pl,maxTS);
    int * ADC_ = smq->Out_ADC(pl,maxTS);
    int * TDC_ = smq->Out_TDC(pl,maxTS);
    int * CID_ = smq->CapID(pl,maxTS);

    std::vector<int>temp1(ADC_,ADC_+maxTS);
    std::vector<int>temp2(TDC_,TDC_+maxTS);
    std::vector<int>temp3(CID_,CID_+maxTS);

    ADCs = temp1;
    TDCs = temp2;
    CIDs = temp3;
  }

  TrigScintQIEDigis::TrigScintQIEDigis(int maxTS_,Pulse* pl, SimQIE* sm)
  {
    maxTS = maxTS_;
    // ADCs = sm->Out_ADC(pl,maxTS);
    // TDCs = sm->Out_TDC(pl,maxTS);
    // CIDs = sm->CapID(pl,maxTS);

    int * ADC_ = sm->Out_ADC(pl,maxTS);
    int * TDC_ = sm->Out_TDC(pl,maxTS);
    int * CID_ = sm->CapID(pl,maxTS);

    std::vector<int>temp1(ADC_,ADC_+maxTS);
    std::vector<int>temp2(TDC_,TDC_+maxTS);
    std::vector<int>temp3(CID_,CID_+maxTS);

    ADCs = temp1;
    TDCs = temp2;
    CIDs = temp3;
  }


  void TrigScintQIEDigis::Print(Option_t* option) const {
    std::cout<<"TrigScintQIEDigis { "
  	     <<"maxTS= "<<maxTS<<", "
  	     <<"chanID= "<<chanID<<", "
  	     <<"ADC[0]= "<<ADCs[0]<<", "
  	     <<"TDC[0]= "<<TDCs[0]<<", "
  	     <<"CID[0]= "<<TDCs[0]<<", "
  	     <<"}\n";
  }

  void TrigScintQIEDigis::Clear(Option_t* option){}
}
// DECLARE_PRODUCER_NS(ldmx, TrigScintQIEDigis);

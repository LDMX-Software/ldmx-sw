#include "Event/dataframe.h"

#include <iostream>
#include <exception>
ClassImp(ldmx::dataframe)

namespace ldmx {
  
  dataframe::dataframe()
  {
  }

  dataframe::~dataframe(){}

  dataframe::dataframe(int maxTS_,Pulse* pl, float pd=0, float ns=0)
  {
    SimQIE* smq;
    if(ns==0) smq = new SimQIE();
    else smq = new SimQIE(pd,ns);
    smq->SetGain();		// by default, 1e+6
    smq->SetFreq();		// by default, 40
    maxTS = maxTS_;
    ADCs = smq->Out_ADC(pl,maxTS);
    TDCs = smq->Out_TDC(pl,maxTS);
    CIDs = smq->CapID(pl,maxTS);
  }

  dataframe::dataframe(int maxTS_,Pulse* pl, SimQIE* sm)
  {
    maxTS = maxTS_;
    ADCs = sm->Out_ADC(pl,maxTS);
    TDCs = sm->Out_TDC(pl,maxTS);
    CIDs = sm->CapID(pl,maxTS);
  }


  void dataframe::Print(Option_t* option) const {
    std::cout<<"dataframe { "
	     <<"maxTS= "<<maxTS<<", "
	     <<"chanID= "<<chanID<<", "
	     <<"ADC[0]= "<<ADCs[0]<<", "
	     <<"TDC[0]= "<<TDCs[0]<<", "
	     <<"CID[0]= "<<TDCs[0]<<", "
	     <<"}\n";
  }

  void dataframe::Clear(Option_t* option){}



  digiCollection::digiCollection()
  {
    int x=0;
  }
}
// DECLARE_PRODUCER_NS(ldmx, dataframe);

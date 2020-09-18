#include "EventProc/dataframe.h"

#include <iostream>
#include <exception>

namespace ldmx {
  dataframe::dataframe()
  {
  }

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

  digiCollection::digiCollection()
  {
    int x=0;
  }
}
// DECLARE_PRODUCER_NS(ldmx, dataframe);

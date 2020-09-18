// Classes which are required to store output of QIE.
// For more information, visit https://github.com/GNiramay/QIE_Simulation/tree/test_dataframe_2
// Class dataframe: to organise output of QIE per event
// Class digiCollection: to store collect digis per event
#include"SimQIE.h"

namespace ldmx {
  class dataframe
  {
  public:
    /* dataframe(int maxTS_,Pulse* pl, float pd=0, float ns=0); */
    dataframe(int maxTS_,Pulse* pl, float pd, float ns);
    dataframe(int maxTS_,Pulse* pl, SimQIE* sm);
    dataframe();

    int* GetADC(){return(ADCs);}
    int* GetTDC(){return(TDCs);}
    int* GetCID(){return(CIDs);}
  
    // private:
    int maxTS;			// no. of time samples stored
    int chanID;			// channel ID

    int* ADCs;			// analog to digital counts
    int* TDCs;			// Time to Digital counts
    int* CIDs;			// capacitor IDs
  };

  class digiCollection
  {
  public:
    digiCollection();
  
    std::vector<dataframe> digis;	/* reco hits maybe */
  };
//   /////////////////////////////////////////////////////////////////////////

//   dataframe::dataframe(){}
//   dataframe::dataframe(int maxTS_,Pulse* pl, float pd=0, float ns=0)
//     {
//       SimQIE* smq;
//       if(ns==0) smq = new SimQIE();
//       else smq = new SimQIE(pd,ns);
//       smq->SetGain();		// by default, 1e+6
//       smq->SetFreq();		// by default, 40
//       maxTS = maxTS_;
//       ADCs = smq->Out_ADC(pl,maxTS);
//       TDCs = smq->Out_TDC(pl,maxTS);
//       CIDs = smq->CapID(pl,maxTS);
//     }

//   dataframe::dataframe(int maxTS_,Pulse* pl, SimQIE* sm)
//     {
//       maxTS = maxTS_;
//       ADCs = sm->Out_ADC(pl,maxTS);
//       TDCs = sm->Out_TDC(pl,maxTS);
//       CIDs = sm->CapID(pl,maxTS);
//     }

//   digiCollection::digiCollection(){}
}

// DECLARE_PRODUCER_NS(ldmx, dataframe);

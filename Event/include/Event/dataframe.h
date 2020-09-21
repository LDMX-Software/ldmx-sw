// Classes which are required to store output of QIE.
// For more information, visit https://github.com/GNiramay/QIE_Simulation/tree/test_dataframe_2
// Class dataframe: to organise output of QIE per event
// Class digiCollection: to store collect digis per event

#ifndef EVENT_DATAFRAME_H
#define EVENT_DATAFRAME_H

#include"SimQIE.h"

namespace ldmx {
  class dataframe
  {
  public:
    dataframe(int maxTS_,Pulse* pl, float pd, float ns);
    dataframe(int maxTS_,Pulse* pl, SimQIE* sm);
    dataframe();

    int* GetADC(){return(ADCs);}
    int* GetTDC(){return(TDCs);}
    int* GetCID(){return(CIDs);}

    void Print(Option_t *option = "") const; // required by Event/include/Event/EventDef.h
    void Clear(Option_t *option = ""); // required by Event/include/Event/EventDef.h
  
    // private:
    int maxTS;			// no. of time samples stored
    int chanID;			// channel ID

    int* ADCs;			// analog to digital counts
    int* TDCs;			// Time to Digital counts
    int* CIDs;			// capacitor IDs
    /* ClassDef(dataframe,1); */
  };

  class digiCollection
  {
  public:
    digiCollection();
  
    std::vector<dataframe> digis;	/* reco hits maybe */
  };
}

// DECLARE_PRODUCER_NS(ldmx, dataframe);
#endif

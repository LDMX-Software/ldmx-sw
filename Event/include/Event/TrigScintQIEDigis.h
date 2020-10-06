#ifndef EVENT_TRIGSCINTQIEDIGIS_H
#define EVENT_TRIGSCINTQIEDIGIS_H

#include"SimQIE.h"
#include "TObject.h" //For ClassDef

namespace ldmx {
  class TrigScintQIEDigis
  {
  public:
    TrigScintQIEDigis(int maxTS_,Pulse* pl, float pd, float ns);
    TrigScintQIEDigis(int maxTS_,Pulse* pl, SimQIE* sm);
    TrigScintQIEDigis();
    ~TrigScintQIEDigis(){};

    std::vector<int> GetADC(){return(ADCs);}
    std::vector<int> GetTDC(){return(TDCs);}
    std::vector<int> GetCID(){return(CIDs);}

    void Print(Option_t *option = "") const; // required by Event/include/Event/EventDef.h
    void Clear(Option_t *option = ""); // required by Event/include/Event/EventDef.h
  
    bool operator < ( const TrigScintQIEDigis &rhs ) const
    { return this->chanID < rhs.chanID;} // required for declaring std::vector<> in Event/include/Event/EventDef.h

    // private:
    int maxTS;			// no. of time samples stored
    int chanID;			// channel ID
    int truePE;			// Net input no. of PEs
    bool IsNoisy;		// Whether or not 

    std::vector<int> ADCs;			// analog to digital counts
    std::vector<int> TDCs;			// Time to Digital counts
    std::vector<int> CIDs;			// capacitor IDs
    ClassDef(TrigScintQIEDigis,1);
  };
}
#endif

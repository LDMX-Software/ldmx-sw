/**
 * @file TrigScintQIEDigis.h
 * @brief class for storing QIE output
 * @author Niramay Gogate, Texas Tech University
 */

#ifndef EVENT_TRIGSCINTQIEDIGIS_H
#define EVENT_TRIGSCINTQIEDIGIS_H

#include "TObject.h" //For ClassDef

namespace ldmx {

  /**
   * @class TrigScintQIEDigis
   * @brief class for storing QIE output
   */
  class TrigScintQIEDigis
  {
  public:

    /**
     * constructor
     * @param maxTS_ no. of time samples to be digitized
     */
    TrigScintQIEDigis(int maxTS_);

    /**
     * Default constructor
     */
    TrigScintQIEDigis(){};
  
    /**
     * Default destructor
     */
    ~TrigScintQIEDigis(){};
  
    /**
     * Print ifo about the class
     * @note required by Event/include/Event/EventDef.h
     */
    void Print(Option_t *option = "") const;

    /**
     * A dummy function
     * @note required by Event/include/Event/EventDef.h
     */
    void Clear(Option_t *option = "");

    /**
     * A dummy operator overloading
     * @note required for declaring std::vector<> in Event/include/Event/EventDef.h
     */  
    bool operator < ( const TrigScintQIEDigis &rhs ) const
    { return this->chanID < rhs.chanID;}

    /// no. of time samples stored
    int maxTS;

    /**
     * Get ADCs of all time samples
     */
    std::vector<int> GetADC(){return(ADCs);}

    /**
     * Get TDCs of all time samples
     */
    std::vector<int> GetTDC(){return(TDCs);}

    /**
     * Get Cap IDs of all time samples
     */
    std::vector<int> GetCID(){return(CIDs);}

    /**
     * Store ADCs of all time samples
     * @param adc_ array of ADCs
     */
    void SetADC(int* adc_);

    /**
     * Store TDCs of all time samples
     * @param tdc_ array of TDCs
     */
    void SetTDC(int* tdc_);

    /**
     * Store CIDs of all time samples
     * @param cid_ array of CIDs
     */
    void SetCID(int* cid_);

    // private:
    /// channel ID
    int chanID;
    /// Net input no. of PEs
    int truePE;
    /// Whether or not there is noise
    bool IsNoisy; 

    /// analog to digital counts
    std::vector<int> ADCs;
    /// Time to Digital counts
    std::vector<int> TDCs;
    /// capacitor IDs
    std::vector<int> CIDs;

    ClassDef(TrigScintQIEDigis,1);
  };
}
#endif

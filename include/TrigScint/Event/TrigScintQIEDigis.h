/**
 * @file TrigScintQIEDigis.h
 * @brief class for storing QIE output
 * @author Niramay Gogate, Texas Tech University
 */

#ifndef TRIGSCINT_EVENT_TRIGSCINTQIEDIGIS_H
#define TRIGSCINT_EVENT_TRIGSCINTQIEDIGIS_H

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
     * Default constructor
     */
    TrigScintQIEDigis(){};
  
    /**
     * Default destructor
     */
    ~TrigScintQIEDigis(){};
  
    /**
     * Print ifo about the class
     * @note required by EventDef.h
     */
    void Print(Option_t *option = "") const;

    /**
     * A dummy function
     * @note required by Event/include/Event/EventDef.h
     */
    void Clear(Option_t *option = "");

    /**
     * A dummy operator overloading
     * @note required for declaring std::vector<> in EventDef.h
     */  
    bool operator < ( const TrigScintQIEDigis &rhs ) const
    { return this->chanID < rhs.chanID;}

    /**
     * Get ADCs of all time samples
     */
    std::vector<int> GetADC(){return(adcs);}

    /**
     * Get tdcs of all time samples
     */
    std::vector<int> GetTDC(){return(tdcs);}

    /**
     * Get Cap IDs of all time samples
     */
    std::vector<int> GetCID(){return(cids);}

    /**
     * Store adcs of all time samples
     * @param adc_ array of adcs
     */
    void SetADC(std::vector<int> adc_) {adcs = adc_;}

    /**
     * Store tdcs of all time samples
     * @param tdc_ array of tdcs
     */
    void SetTDC(std::vector<int> tdc_) {tdcs = tdc_;}

    /**
     * Store cids of all time samples
     * @param cid_ array of cids
     */
    void SetCID(std::vector<int> cid_) {cids = cid_;}

    /// channel ID
    int chanID;
    /// Net input no. of PEs
    int truePE;
    /// Whether or not there is noise
    bool IsNoisy; 

    /// analog to digital counts
    std::vector<int> adcs;
    /// Time to Digital counts
    std::vector<int> tdcs;
    /// capacitor IDs
    std::vector<int> cids;

    ClassDef(TrigScintQIEDigis,1);
  };
}
#endif

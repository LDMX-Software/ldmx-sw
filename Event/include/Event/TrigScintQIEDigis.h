/**
 * @file TrigScintQIEDigis.h
 * @brief class for storing QIE output
 * @author Niramay Gogate, Texas Tech University
 */

#ifndef EVENT_TRIGSCINTQIEDIGIS_H
#define EVENT_TRIGSCINTQIEDIGIS_H

#include"SimQIE.h"
#include "TObject.h" //For ClassDef

namespace ldmx {

/**
 * @class SimQIE
 * @brief class for simulating QIE chip output
 * @note This should be initialized only once per simulation
 */
class TrigScintQIEDigis
{
 public:

  /**
   * constructor
   * @param maxTS_ no. of time samples to be digitized
   * @param pl instance of pulse or its daughters
   * @param pd pedestal level
   * @param ns noise level
   * 
   * @note SimQIE instance is initialized automatically
   */
  TrigScintQIEDigis(int maxTS_,Pulse* pl, float pd, float ns);

  /**
   * Defaut constructor
   * @param maxTS_ no. of time samples to be digitized
   * @param pl instance of pulse or its daughters
   * @param sm instance of SimQIE
   *
   * @note The most preferred way of initialization
   */
  TrigScintQIEDigis(int maxTS_,Pulse* pl, SimQIE* sm);

  /**
   * Default constructor
   */
  TrigScintQIEDigis();

  /**
   * Default constructor
   */
  ~TrigScintQIEDigis(){};

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

  // private:
  // no. of time samples stored
  int maxTS;
  // channel ID
  int chanID;
  // Net input no. of PEs
  int truePE;
  // Whether or not
  bool IsNoisy; 

  // analog to digital counts
  std::vector<int> ADCs;
  // Time to Digital counts
  std::vector<int> TDCs;
  // capacitor IDs
  std::vector<int> CIDs;

  ClassDef(TrigScintQIEDigis,1);
};
}
#endif

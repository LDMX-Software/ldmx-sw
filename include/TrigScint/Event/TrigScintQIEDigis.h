#ifndef TRIGSCINT_EVENT_TRIGSCINTQIEDIGIS_H
#define TRIGSCINT_EVENT_TRIGSCINTQIEDIGIS_H

//---< ROOT >---//
#include "TObject.h" 

namespace trigscint {

/**
 * @class TrigScintQIEDigis
 * @brief class for storing QIE output
 */
class TrigScintQIEDigis {
 public:
  
   /// Default constructor
  TrigScintQIEDigis() = default;

  /// Default destructor
  ~TrigScintQIEDigis() = default;

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
  bool operator<(const TrigScintQIEDigis &rhs) const {
    return this->chanID_ < rhs.chanID_;
  }

  /**
   * Get channel ID
   */
  int getChanID() const { return chanID_; }

  /**
   * Get ADCs of all time samples
   */
  std::vector<int> getADC() const { return adcs_; }

  /**
   * Get tdcs of all time samples
   */
  std::vector<int> getTDC() const { return tdcs_; }

  /**
   * Get Cap IDs of all time samples
   */
  std::vector<int> getCID() const { return cids_; }

  /**
   * Store the channel ID
   */
  void setChanID(const int chanid) { chanID_ = chanid; }

  /**
   * Store adcs of all time samples
   * @param adc_ array of adcs
   */
  void setADC(const std::vector<int> adc) { adcs_ = adc; }

  /**
   * Store tdcs of all time samples
   * @param tdc_ array of tdcs
   */
  void setTDC(const std::vector<int> tdc) { tdcs_ = tdc; }

  /**
   * Store cids of all time samples
   * @param cid_ array of cids
   */
  void setCID(const std::vector<int> cid) { cids_ = cid; }

protected:
  /// channel ID
  int chanID_;

  /// analog to digital counts
  std::vector<int> adcs_;
  
  /// Time to Digital counts
  std::vector<int> tdcs_;
  
 private:
  /// capacitor IDs
  std::vector<int> cids_;

  ClassDef(TrigScintQIEDigis, 1);
};
}  // namespace trigscint
#endif

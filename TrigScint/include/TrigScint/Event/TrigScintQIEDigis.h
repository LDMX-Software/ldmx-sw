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
  virtual ~TrigScintQIEDigis() = default;

  /**
   * Print ifo about the class
   */
  friend std::ostream &operator<<(std::ostream &o, const TrigScintQIEDigis &d);

  /**
   * A dummy function
   * @note required by Event/include/Event/EventDef.h
   */
  void clear(Option_t *option = "");

  /**
   * A dummy operator overloading
   * @note required for declaring std::vector<> in EventDef.h
   */
  bool operator<(const TrigScintQIEDigis &rhs) const {
    return this->chan_id_ < rhs.chan_id_;
  }

  /**
   * Get channel ID
   */
  int getChanID() const { return chan_id_; }

  /**
   * Get electronics ID
   */
  int getElecID() const { return elec_id_; }

  /**
   * Get module ID (pad naming number -1)
   */
  int getModuleID() const { return module_id_; }

  /**
   * Get lane ID
   */
  int getLaneID() const { return lane_id_; }

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
   * Get BC0s (= 1 for periodic sync time sample) of all time samples
   */
  std::vector<int> getBC0() const { return bc0s_; }
  
  /**
   * Get CEs (an error state flag) of all time samples
   */
  std::vector<int> getCE() const { return ces_; }
  
  /**
   * Store the event time since spill counter
   */
  void setTimeSinceSpill(const uint32_t timeSpill) {
    time_since_spill_counter_ = timeSpill;
  }

  /**
   * Store the event time since spill counter
   */
  uint32_t getTimeSinceSpill() const { return time_since_spill_counter_; }

  /**
   * Store the channel ID
   */
  void setChanID(const int chanid) { chan_id_ = chanid; }

  /**
   * Store the electronics ID
   */
  void setElecID(const int elecid) { elec_id_ = elecid; }

  /**
   * Store the module ID (pad number-1)
   */
  void setModuleID(const int moduleid) { module_id_ = moduleid; }

  /**
   * Store the lane (fiber) ID
   */
  void setLaneID(const int laneid) { lane_id_ = laneid; }

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

  /**
   * Store bc0s of all time samples
   * @param bc0_ array of BC0s (perodic sample sync)
   */
  void setBC0(const std::vector<int> bc0) { bc0s_ = bc0; }

  /**
   * Store ces of all time samples
   * @param ce_ array of CEs (error state)
   */
  void setCE(const std::vector<int> ce) { ces_ = ce; }

  
  
 protected:
  /// channel ID
  int chan_id_;
  /// channel ID
  int elec_id_{-1};
  /// module ID (pad)
  int module_id_;
  /// lane ID (fiber)
  int lane_id_{-1};

  /// analog to digital counts
  std::vector<int> adcs_;

  /// Time to Digital counts
  std::vector<int> tdcs_;

  /// Time since spill (a counter, to be divided by 125e6 or so)
  uint32_t time_since_spill_counter_;

 private:
  /// capacitor IDs
  std::vector<int> cids_;
  std::vector<int> bc0s_;
  std::vector<int> ces_;

  ClassDef(TrigScintQIEDigis, 4);
};
}  // namespace trigscint
#endif

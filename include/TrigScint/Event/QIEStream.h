#ifndef TRIGSCINT_EVENT_QIESTREAM_H
#define TRIGSCINT_EVENT_QIESTREAM_H

//---< ROOT >---//
#include "TObject.h" 
#include "TrigScint/Event/TrigScintQIEDigis.h"

template <uint32_t N>
struct mask {
  static const uint32_t one{1};
  static const uint32_t m = (one << N) - one;
};
template <uint8_t n>
struct mask8 {
  static const uint8_t one{1};
  static const uint8_t m = (one << n) - one;
};


namespace trigscint {

/**
 * @class QIEStream
 * @brief class for storing QIE output as a binary stream
 */
class QIEStream {
 public:
  
   /// Default constructor
  QIEStream() = default;

  /// Default destructor
  ~QIEStream() = default;

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
  bool operator<(const QIEStream &rhs) const {
    return this->chanID_ < rhs.chanID_;
  }

  /**
   * Get channel ID
   */
  int getChannelID() const { return chanID_; }

  /**
   * Get electronics ID
   */
  uint8_t getElectronicsID() const { return electronicsID_; }

  /**
   * Get ADCs of all time samples
   */
  std::vector<int> getDecodedADC() const { return adcs_; }

  /**
   * Get TDCs of all time samples
   */
  std::vector<int> getDecodedTDC() const { return tdcs_; }


  /**
   * Get Cap IDs of all time samples
   */
  std::vector<int> getDecodedCID() const { return cids_; }


  /**
   * Store the channel ID
   */
  void setChannelID(const int chanid) { chanID_ = chanid; }

  /**
   * Store the electronics ID
   */
  void setElectronicsID(const int elecid) { electronicsID_ = elecid; }

  /**
   * Store adcs of all time samples
   * @param adc_ array of adcs
   */
  void setDecodedADC(const std::vector<int> adc) { adcs_ = adc; }

  /**
   * Store tdcs of all time samples
   * @param tdc_ array of tdcs
   */
  void setDecodedTDC(const std::vector<int> tdc) { tdcs_ = tdc; }


  /**
   * Store cids of all time samples
   * @param cid_ array of cids
   */
  void setDecodedCID(const std::vector<int> cid) { cids_ = cid; }


public:

  //use these to define positions and sizes of error flags etc other header words
  const static int TRIGID_POS{0};
  const static int ERROR_POS{1};
  const static int CHECKSUM_POS{2};
  // and positions of error bits/flags in the errors word 
  const static int FLAG_SIZE{1};
  const static int CRC0_ERR_POS{0};
  const static int CRC1_ERR_POS{1};
  const static int CID_UNSYNC_POS{2};
  const static int CID_SKIP_POS{3};
  const static int NUM_SAMPLES{5};  //apparently only 5      

private:
  /// detector channel ID (bar nb)
  int chanID_;
  /// electronics ID
  int electronicsID_;

  /// Analog to Digital counts
  std::vector<int> adcs_;
  /// Time to Digital counts
  std::vector<int> tdcs_;
  /// Capacitor IDs
  std::vector<int> cids_;

  
  ClassDef(QIEStream, 1);
};
}  // namespace trigscint
#endif

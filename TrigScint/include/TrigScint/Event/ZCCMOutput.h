#ifndef TRIGSCINT_EVENT_ZCCMOUTPUT_H
#define TRIGSCINT_EVENT_ZCCMOUTPUT_H

//---< ROOT >---//
#include "TObject.h"
#include "TrigScint/Event/TrigScintQIEDigis.h"
#include "QIEStream.h" // for definitions of Mask

/*
  template <uint32_t N>
struct Mask {
  static const uint32_t ONE{1};
  static const uint32_t M = (ONE << N) - ONE;
};
template <uint8_t n>
struct Mask8 {
  static const uint8_t ONE{1};
  static const uint8_t M = (ONE << n) - ONE;
};
*/
namespace trigscint {

/**
 * @class ZCCMOutput
 * @brief class for storing ZCCM output as a binary output
 */
class ZCCMOutput {
 public:
  /// Default constructor
  ZCCMOutput() = default;

  /// Default destructor
  virtual ~ZCCMOutput() = default;

  /**
   * Print ifo about the class
   */
  friend std::ostream &operator<<(std::ostream &o, const ZCCMOutput &d);

  /**
   * A dummy function
   * @note required by Event/include/Event/EventDef.h
   */
  void clear(Option_t *option = "");

  /**
   * A dummy operator overloading
   * @note required for declaring std::vector<> in EventDef.h
   */
  bool operator<(const ZCCMOutput &rhs) const {
    return this->chan_id_ < rhs.chan_id_;
  }

  /**
   * Get channel ID
   */
  int getChannelID() const { return chan_id_; }

  /**
   * Get electronics ID
   */
  uint8_t getElectronicsID() const { return electronics_id_; }

  /**
   * Get ADCs of all time samples
   */
  std::vector<int> getADC() const { return adcs_; }

  /**
   * Get TDCs of all time samples
   */
  std::vector<int> getTDC() const { return tdcs_; }

  /**
   * Get Cap IDs of all time samples
   */
  std::vector<int> getCID() const { return cids_; }

  /**
   * Store the channel ID
   */
  void setChannelID(const int chanid) { chan_id_ = chanid; }

  /**
   * Store the electronics ID
   */
  void setElectronicsID(const int elecid) { electronics_id_ = elecid; }

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

 public:
  /*
    use these to define positions and sizes of error flags etc other header
    words relative to each other.
    these simply state the order:
    header {16B DAQ header, 8B TS time stamp, 8B empty}{message 0}...{message N-1}
    for each time sample, every lane each sends a message:
    [ 1B ADC for 6 channels, 2B empty, 1B TDC for 6 channels, 1B flags, 1B lane nb];
    and the 1B "flags" structure is:
    flags = 2b CID (capID), 1b CE (channel alignment? error), 1b BC0, 4b empty
  */
  const static int DAQHEADER_POS{0};
  const static int DAQHEADER_LEN_BYTES{16}; 
  const static int TIMESTAMP_POS{DAQHEADER_POS + DAQHEADER_LEN_BYTES};
  const static int TIMESTAMP_LEN_BYTES{8};
  const static int EMPTY_HEADER_WORD_POS{TIMESTAMP_POS +
					 TIMESTAMP_LEN_BYTES};
  const static int EMPTY_HEADER_WORD_LEN_BYTES{8};
  // this marks the start of event data (i.e. after end of event header)
  const static int EVENTDATA_POS{EMPTY_HEADER_WORD_POS +
				 EMPTY_HEADER_WORD_LEN_BYTES};
  /*
    the time sample word has ADC, TDC, flags and lane:
    one for each lane, in every time sample
    the following positions are within this time sample word, for one lane
  */
  // the number of channels sets the length of the ADC and TDC streams
  const static int NUM_CHAN_PER_LANE{6};
  const static int ADC_SAMPLE_WORD_POS{0};
  const static int ADC_LEN_BYTES{1*NUM_CHAN_PER_LANE};
  const static int EMPTY_WORD_SAMPLE_WORD_POS{ADC_SAMPLE_WORD_POS+ADC_LEN_BYTES};
  const static int EMPTY_WORD_LEN_BYTES{2};
  const static int TDC_SAMPLE_WORD_POS{EMPTY_WORD_SAMPLE_WORD_POS
				       +EMPTY_WORD_LEN_BYTES};
  const static int TDC_LEN_BYTES{1*NUM_CHAN_PER_LANE};
  const static int FLAGS_SAMPLE_WORD_POS{TDC_SAMPLE_WORD_POS+TDC_LEN_BYTES};
  const static int FLAGS_LEN_BYTES{1};
  const static int LANE_SAMPLE_WORD_POS{FLAGS_SAMPLE_WORD_POS + FLAGS_LEN_BYTES};
  const static int LANE_LEN_BYTES{1};
  // and positions of error bits/flags inside the flags word
  const static int CAPID_POS_IN_FLAG{0};
  const static int CAPID_LEN_BITS{2};
  const static int CE_POS_IN_FLAG{CAPID_POS_IN_FLAG + CAPID_LEN_BITS};
  const static int CE_LEN_BITS{1};
  const static int BC0_POS_IN_FLAG{CE_POS_IN_FLAG + CE_LEN_BITS};
  const static int BC0_LEN_BITS{1};
  const static int EMPTY_FLAG_WORD_POS_IN_FLAG{BC0_POS_IN_FLAG+BC0_LEN_BITS};
  const static int EMPTY_FLAG_WORD_LEN_BITS{4};
  
  // event data concludes the readout.

  // for convenience, define the length of a message (time sample for one lane)
  const static int SAMPLE_WORD_LEN_BYTES = ADC_LEN_BYTES + TDC_LEN_BYTES
                                       + EMPTY_WORD_LEN_BYTES + FLAGS_LEN_BYTES
                                       + LANE_LEN_BYTES ;
 
 private:
  /// detector lane ID (aka fiber)
  int lane_;
  /// detector module ID (aka pad)
  int module_;
  /// detector channel ID (bar nb)
  int chan_id_;
  /// electronics ID
  int electronics_id_;
  /// Analog to Digital counts
  std::vector<int> adcs_;
  /// Time to Digital counts
  std::vector<int> tdcs_;
  /// Capacitor IDs
  std::vector<int> cids_;

  ClassDef(ZCCMOutput, 1);
};
}  // namespace trigscint
#endif


#include <bitset>
#include <iomanip>
#include <optional>

#include "Framework/EventProcessor.h"
#include "Packing/Utility/Mask.h"
#include "Packing/Utility/Reader.h"

// un comment for FiberTrackerRawDecoder-specific debug printouts to std::cout
#define DEBUG

namespace packing {

/**
 * converstion from 64-bit int to double specific to FiberTrackerDAQ
 */
double to_double_ft(uint64_t i) {
  static const unsigned int bits = 32;
  static const unsigned int expbits = 8;
  long double result;
  long long shift;
  unsigned bias;
  unsigned significandbits = bits - expbits - 1; // -1 for sign bit

  if (i == 0) return 0.0;

  // pull the significand
  result = (i&((1LL<<significandbits)-1)); // mask
  result /= (1LL<<significandbits); // convert back to float
  result += 1.0f; // add the one back on

  // deal with the exponent
  bias = (1<<(expbits-1)) - 1;
  shift = ((i>>significandbits)&((1LL<<expbits)-1)) - bias;
  while(shift > 0) { result *= 2.0; shift--; }
  while(shift < 0) { result /= 2.0; shift++; }

  // sign it
  result *= (i>>(bits-1))&1? -1.0: 1.0;

  return result;
}

/**
 * Each "field" of data in a FiberTracker packet
 */
class FiberTrackerField {
  uint32_t field_header_;
  std::vector<uint32_t> field_value_;
 public:
  /**
   * r - reader
   * i_field - field we are supposed to be reading from
   */
  FiberTrackerField(utility::Reader& r, int i_field) {
    uint32_t len;
    r >> len >> field_header_;
    r.read(field_value_,len-1);
    if (i_field != field_header_) {
      EXCEPTION_RAISE("BadForm", "Field "+std::to_string(i_field)+" has a mismatched header "+std::to_string(field_header_));
    }
  }

  /**
   * conversion to a single int
   */
  int to_int(const std::size_t i = 0) const {
    return field_value_.at(i);
  }

  /**
   * conversion from series of ints to string specific to FiberTrackerDAQ
   */
  std::string to_string() const {
    std::string str;
    str.resize(field_value_.size());
    for (int i{0}; i < str.size(); i++) {
      str[i] = (char)field_value_[i];
    }
    return str;
  }
  
  /**
   * long split across two ints
   *
   * i is index of field value to start from
   */
  long int to_long(const std::size_t i = 0) const {
    return ((uint64_t)field_value_.at(i+1) << 32) | (uint64_t)field_value_.at(i);
  }

  /**
   * convert two ints into a double
   */
  double to_double(const std::size_t i = 0) const {
    return to_double_ft(this->to_long(i));
  }

  /**
   * Get the field value
   */
  const std::vector<uint32_t>& value() const {
    return field_value_;
  }

};

/**
 * A spill of events from a FiberTracker station
 *
 * Each event is 10 32-bit words where
 *
 * Word 0: Timestamp LSB in 8 ns ticks since the last whole second, for the trigger signal
 * Word 1: Timestamp MSB in UNIX epoch seconds for the trigger signal (in international atomic time)
 *
 * Word 2: Timestamp LSB in 8 ns ticks since the last whole second, for the "event" 
 * Word 3: Timestamp MSB in UNIX epoch seconds for the "event"
 *  (honestly don't know what they mean by "event", so I ignore this timestamp)
 *
 * Word 4-9: Hit information, the fiber tracker has 6*32=192 fibers, a 1 means a hit and a 0 means no hit. 
 *
 * There are 192 bits to make the system compatible with the samll and big detectors.
 * In T9 we have small detectors (only 96 channels) so we have lots of zeros.
 * Only channels 48-144 have detector data, the other channels are empty (all zeros).
 */
struct FiberTrackerEvent {
  int trigger_timestamp_lsb,
      trigger_timestamp_msb,
      event_timestamp_lsb,
      event_timestamp_msb;
  std::vector<uint32_t> channel_hits;

  FiberTrackerEvent() = default;

  FiberTrackerEvent(const std::vector<uint32_t>& spill_data, std::size_t i_word) {
    trigger_timestamp_lsb = spill_data.at(i_word);
    trigger_timestamp_msb = spill_data.at(i_word+1);
    event_timestamp_lsb   = spill_data.at(i_word+2);
    event_timestamp_msb   = spill_data.at(i_word+3);
    channel_hits.clear();
    channel_hits.reserve(6);
    for (std::size_t i{i_word+4}; i < i_word+10 and i < spill_data.size(); i++)
      channel_hits.push_back(spill_data.at(i));
  }
};

/**
 * Each one of these packets represents an entire spill of data
 */
struct FiberTrackerBinaryPacket {
  int acqMode;
  long int acqStamp;
  int acqType;
  int acqTypeAllowed;
  std::string coincidenceInUse;
  int counts;
  long int countsRecords;
  long int countsRecordsWithZeroEvents;
  long int countsTrigs;
  std::string cycleName;
  long int cycleStamp;
  std::string equipmentName;
  int eventSelectionAcq;
  /**
   * This is the actual event data in which we are interested
   */
  std::vector<FiberTrackerEvent> eventsData;
  /// index of event we are on (for next)
  int i_event{0};

  double meanSNew;
  std::string message;
  std::vector<double> profile;
  std::vector<double> profileStandAlone;
  std::string timeFirstEvent;
  std::string timeFirstTrigger;
  std::string timeLastEvent;
  std::string timeLastTrigger;
  int trigger;
  int triggerOffsetAcq;
  int triggerSelectionAcq;

  bool next(FiberTrackerEvent& e) {
    i_event++;
    if (i_event < eventsData.size()) {
      e = eventsData.at(i_event);
      return true;
    }
    return false;
  }

  /**
   * 27 fields in order, all are present
   */
  utility::Reader& read(utility::Reader& r) {
    int i_field{0};
    acqMode = FiberTrackerField(r,++i_field).to_int();
#ifdef DEBUG
    std::cout << i_field << " " << "acqMode = " << acqMode << std::endl;
#endif
    acqStamp = FiberTrackerField(r,++i_field).to_long();
#ifdef DEBUG
    std::cout << i_field << " " << "acqStamp = " << acqStamp << std::endl;
#endif
    acqType = FiberTrackerField(r,++i_field).to_int();
#ifdef DEBUG
    std::cout << i_field << " " << "acqType = " << acqType << std::endl;
#endif
    acqTypeAllowed = FiberTrackerField(r,++i_field).to_int();
#ifdef DEBUG
    std::cout << i_field << " " << "acqTypeAllowed = " << acqTypeAllowed << std::endl;
#endif
    coincidenceInUse = FiberTrackerField(r,++i_field).to_string();
#ifdef DEBUG
    std::cout << i_field << " " << "coincidenceInUse = " << coincidenceInUse << std::endl;
#endif
    counts = FiberTrackerField(r,++i_field).to_int();
#ifdef DEBUG
    std::cout << i_field << " " << "counts = " << counts << std::endl;
#endif
    countsRecords = FiberTrackerField(r,++i_field).to_long();
#ifdef DEBUG
    std::cout << i_field << " " << "countsRecords = " << countsRecords << std::endl;
#endif
    countsRecordsWithZeroEvents = FiberTrackerField(r,++i_field).to_long();
#ifdef DEBUG
    std::cout << i_field << " " << "countsRecordsWithZeroEvents = " << countsRecordsWithZeroEvents << std::endl;
#endif
    countsTrigs = FiberTrackerField(r,++i_field).to_long();
#ifdef DEBUG
    std::cout << i_field << " " << "countsTrigs = " << countsTrigs << std::endl;
#endif
    cycleName = FiberTrackerField(r,++i_field).to_string();
#ifdef DEBUG
    std::cout << i_field << " " << "cycleName = " << cycleName << std::endl;
#endif
    cycleStamp = FiberTrackerField(r,++i_field).to_long();
#ifdef DEBUG
    std::cout << i_field << " " << "cycleStamp = " << cycleStamp << std::endl;
#endif
    equipmentName = FiberTrackerField(r,++i_field).to_string();
#ifdef DEBUG
    std::cout << i_field << " " << "equipmentName = " << equipmentName << std::endl;
#endif
    eventSelectionAcq = FiberTrackerField(r,++i_field).to_int();
#ifdef DEBUG
    std::cout << i_field << " " << "eventSelectionAcq = " << eventSelectionAcq << std::endl;
#endif
    FiberTrackerField events_data_field(r,++i_field);
    i_event = -1;
    eventsData.clear();
    eventsData.reserve( events_data_field.value().size()/10 );
    for (std::size_t i_word{0}; i_word < events_data_field.value().size(); i_word += 10) {
      eventsData.emplace_back(events_data_field.value(), i_word);
    }
#ifdef DEBUG
    std::cout << i_field << " " << "eventsData (size = " << eventsData.size() << ")" << std::endl;
#endif
    meanSNew = FiberTrackerField(r,++i_field).to_double();
#ifdef DEBUG
    std::cout << i_field << " " << "meanSNew = " << meanSNew << std::endl;
#endif
    message = FiberTrackerField(r,++i_field).to_string();
#ifdef DEBUG
    std::cout << i_field << " " << "message = " << message << std::endl;
#endif
    FiberTrackerField profile_field(r,++i_field);
    profile.clear();
    profile.reserve( profile_field.value().size()/2 );
    for (std::size_t i_word{0}; i_word < profile_field.value().size(); i_word += 2) {
      profile.push_back(profile_field.to_double(i_word));
    }
#ifdef DEBUG
    std::cout << i_field << " " << "profile size " << profile.size() << std::endl;
#endif
    // fields 18 and 19 are skipped
    i_field += 2;
    FiberTrackerField profileStandAlone_field(r,++i_field);
    profileStandAlone.clear();
    profileStandAlone.reserve( profileStandAlone_field.value().size()/2 );
    for (std::size_t i_word{0}; i_word < profileStandAlone_field.value().size(); i_word += 2) {
      profileStandAlone.push_back(profileStandAlone_field.to_double(i_word));
    }
#ifdef DEBUG
    std::cout << i_field << " " << "profileStandAlone size " << profileStandAlone.size() << std::endl;
#endif
    timeFirstEvent = FiberTrackerField(r,++i_field).to_string();
#ifdef DEBUG
    std::cout << i_field << " " << "timeFirstEvent = " << timeFirstEvent << std::endl;
#endif
    timeFirstTrigger = FiberTrackerField(r,++i_field).to_string();
#ifdef DEBUG
    std::cout << i_field << " " << "timeFirstTrigger = " << timeFirstTrigger << std::endl;
#endif
    timeLastEvent = FiberTrackerField(r,++i_field).to_string();
#ifdef DEBUG
    std::cout << i_field << " " << "timeLastEvent = " << timeLastEvent << std::endl;
#endif
    timeLastTrigger = FiberTrackerField(r,++i_field).to_string();
#ifdef DEBUG
    std::cout << i_field << " " << "timeLastTrigger = " << timeLastTrigger << std::endl;
#endif
    trigger = FiberTrackerField(r,++i_field).to_int();
#ifdef DEBUG
    std::cout << i_field << " " << "trigger = " << trigger << std::endl;
#endif
    triggerOffsetAcq = FiberTrackerField(r,++i_field).to_int();
#ifdef DEBUG
    std::cout << i_field << " " << "triggerOffsetAcq = " << triggerOffsetAcq << std::endl;
#endif
    triggerSelectionAcq = FiberTrackerField(r,++i_field).to_int();
#ifdef DEBUG
    std::cout << i_field << " " << "triggerSelectionAcq = " << triggerSelectionAcq << std::endl;
#endif
    return r;
  } 
};

std::ostream& operator<<(std::ostream& os, const FiberTrackerBinaryPacket& p) {
  return (os << "FiberTracker Packet {" << p.acqStamp << "}");
}

/**
 * @class FiberTrackerRawDecoder
 */
class FiberTrackerRawDecoder : public framework::Producer {
 public:
  FiberTrackerRawDecoder(const std::string& name, framework::Process& process)
      : framework::Producer(name, process) {}
  virtual ~FiberTrackerRawDecoder() = default;
  virtual void configure(framework::config::Parameters&) final override;
  virtual void onProcessStart() final override;
  virtual void produce(framework::Event& event) final override;

 private:
  /// input file
  std::string input_file_;
  /// output object to put onto event bus
  std::string output_name_;
  /// should we ntuplize?
  bool ntuplize_;
 private:
  /// the file reader (if we are doing that)
  packing::utility::Reader file_reader_;
  /// packet being used for decoding
  FiberTrackerBinaryPacket spill_packet_;
  /// Current Event
  FiberTrackerEvent ft_event_;
  /// ntuple tree
  TTree* tree_;
};

void FiberTrackerRawDecoder::configure(framework::config::Parameters& ps) {
  input_file_ = ps.getParameter<std::string>("input_file");
  output_name_ = ps.getParameter<std::string>("output_name");
  ntuplize_ = ps.getParameter<bool>("ntuplize");

  file_reader_.open(input_file_);
}

void FiberTrackerRawDecoder::onProcessStart() {
  if (ntuplize_) {
    getHistoDirectory();
    tree_ = new TTree("raw","Flattened and decoded raw FiberTracker data");
    tree_->Branch("TriggerTSLSB", &ft_event_.trigger_timestamp_lsb);
    tree_->Branch("TriggerTSMSB", &ft_event_.trigger_timestamp_msb);
    tree_->Branch("EventTSLSB", &ft_event_.event_timestamp_lsb);
    tree_->Branch("EventTSMSB", &ft_event_.event_timestamp_msb);
    tree_->Branch("ChannelHits", &ft_event_.channel_hits);
  }
}

void FiberTrackerRawDecoder::produce(framework::Event& event) {
  // only add and fill when file able to readout packet
  if (not spill_packet_.next(ft_event_)) {
    // no more events in this spill
    if (file_reader_ >> spill_packet_) {
      // next spill loaded, pop its first event
      spill_packet_.next(ft_event_);
    } else {
#ifdef DEBUG
      std::cout << "no more events" << std::endl;
#endif
      return;
    }
  }

  event.add(output_name_+"TriggerTSLSB", ft_event_.trigger_timestamp_lsb);
  event.add(output_name_+"TriggerTSMSB", ft_event_.trigger_timestamp_msb);
  event.add(output_name_+"EventTSLSB", ft_event_.event_timestamp_lsb);
  event.add(output_name_+"EventTSMSB", ft_event_.event_timestamp_msb);
  event.add(output_name_+"Hits", ft_event_.channel_hits);
  tree_->Fill();
  return;
}  // produce

}  // namespace packing

DECLARE_PRODUCER_NS(packing, FiberTrackerRawDecoder);

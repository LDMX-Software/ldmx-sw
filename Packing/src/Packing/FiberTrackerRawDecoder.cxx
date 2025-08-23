
#include <bitset>
#include <iomanip>
#include <optional>

#include "Framework/EventProcessor.h"
#include "Packing/Utility/Mask.h"
#include "Packing/Utility/Reader.h"

// un comment for FiberTrackerRawDecoder-specific debug printouts to std::cout
// #define DEBUG

namespace packing {

/**
 * converstion from 64-bit int to double specific to FiberTrackerDAQ
 */
double toDoubleFt(uint64_t i) {
  static const unsigned int bits = 32;
  static const unsigned int expbits = 8;
  long double result;
  long long shift;
  unsigned bias;
  unsigned significandbits = bits - expbits - 1;  // -1 for sign bit

  if (i == 0) return 0.0;

  // pull the significand
  result = (i & ((1LL << significandbits) - 1));  // mask
  result /= (1LL << significandbits);             // convert back to float
  result += 1.0f;                                 // add the one back on

  // deal with the exponent
  bias = (1 << (expbits - 1)) - 1;
  shift = ((i >> significandbits) & ((1LL << expbits) - 1)) - bias;
  while (shift > 0) {
    result *= 2.0;
    shift--;
  }
  while (shift < 0) {
    result /= 2.0;
    shift++;
  }

  // sign it
  result *= (i >> (bits - 1)) & 1 ? -1.0 : 1.0;

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
    r.read(field_value_, len - 1);
    if (i_field != field_header_) {
      EXCEPTION_RAISE("BadForm", "Field " + std::to_string(i_field) +
                                     " has a mismatched header " +
                                     std::to_string(field_header_));
    }
  }

  /**
   * conversion to a single int
   */
  int toInt(const std::size_t i = 0) const { return field_value_.at(i); }

  /**
   * conversion from series of ints to string specific to FiberTrackerDAQ
   */
  std::string toString() const {
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
   * i is index_ of field value to start from
   */
  long int toLong(const std::size_t i = 0) const {
    return ((uint64_t)field_value_.at(i + 1) << 32) |
           (uint64_t)field_value_.at(i);
  }

  /**
   * convert two ints into a double
   */
  double toDouble(const std::size_t i = 0) const {
    return toDoubleFt(this->toLong(i));
  }

  /**
   * Get the field value
   */
  const std::vector<uint32_t>& value() const { return field_value_; }
};

/**
 * A spill of events from a FiberTracker station
 *
 * Each event is 10 32-bit words where
 *
 * Word 0: Timestamp LSB in 8 ns ticks since the last whole second, for the
 * trigger signal Word 1: Timestamp MSB in UNIX epoch seconds for the trigger
 * signal (in international atomic time)
 *
 * Word 2: Timestamp LSB in 8 ns ticks since the last whole second, for the
 * "event" Word 3: Timestamp MSB in UNIX epoch seconds for the "event" (honestly
 * don't know what they mean by "event", so I ignore this timestamp)
 *
 * Word 4-9: Hit information, the fiber tracker has 6*32=192 fibers, a 1 means a
 * hit and a 0 means no hit.
 *
 * There are 192 bits to make the system compatible with the samll and big
 * detectors. In T9 we have small detectors (only 96 channels) so we have lots
 * of zeros. Only channels 48-144 have detector data, the other channels are
 * empty (all zeros).
 */
struct FiberTrackerEvent {
  int trigger_timestamp_lsb_, trigger_timestamp_msb_, event_timestamp_lsb_,
      event_timestamp_msb_;
  std::vector<uint32_t> channel_hits_;

  FiberTrackerEvent() = default;

  FiberTrackerEvent(const std::vector<uint32_t>& spill_data,
                    std::size_t i_word) {
    trigger_timestamp_lsb_ = spill_data.at(i_word);
    trigger_timestamp_msb_ = spill_data.at(i_word + 1);
    event_timestamp_lsb_ = spill_data.at(i_word + 2);
    event_timestamp_msb_ = spill_data.at(i_word + 3);
    channel_hits_.clear();
    channel_hits_.reserve(6);
    for (std::size_t i{i_word + 4}; i < i_word + 10 and i < spill_data.size();
         i++)
      channel_hits_.push_back(spill_data.at(i));
  }
};

/**
 * Each one of these packets represents an entire spill of data
 */
struct FiberTrackerBinaryPacket {
  int acq_mode_;
  long int acq_stamp_;
  int acq_type_;
  int acq_type_allowed_;
  std::string coincidence_in_use_;
  int counts_;
  long int counts_records_;
  long int counts_records_with_zero_events_;
  long int counts_trigs_;
  std::string cycle_name_;
  long int cycle_stamp_;
  std::string equipment_name_;
  int event_selection_acq_;
  /**
   * This is the actual event data in which we are interested
   */
  std::vector<FiberTrackerEvent> events_data_;
  /// index_ of event we are on (for next)
  int i_event_{0};

  double mean_s_new_;
  std::string message_;
  std::vector<double> profile_;
  std::vector<double> profile_stand_alone_;
  std::string time_first_event_;
  std::string time_first_trigger_;
  std::string time_last_event_;
  std::string time_last_trigger_;
  int trigger_;
  int trigger_offset_acq_;
  int trigger_selection_acq_;

  bool next(FiberTrackerEvent& e) {
    i_event_++;
    if (i_event_ < events_data_.size()) {
      e = events_data_.at(i_event_);
      return true;
    }
    return false;
  }

  /**
   * 27 fields in order, all are present
   */
  utility::Reader& read(utility::Reader& r) {
    int i_field{0};
    acq_mode_ = FiberTrackerField(r, ++i_field).toInt();
#ifdef DEBUG
    std::cout << i_field << " " << "acqMode = " << acqMode << std::endl;
#endif
    acq_stamp_ = FiberTrackerField(r, ++i_field).toLong();
#ifdef DEBUG
    std::cout << i_field << " " << "acqStamp = " << acqStamp << std::endl;
#endif
    acq_type_ = FiberTrackerField(r, ++i_field).toInt();
#ifdef DEBUG
    std::cout << i_field << " " << "acqType = " << acqType << std::endl;
#endif
    acq_type_allowed_ = FiberTrackerField(r, ++i_field).toInt();
#ifdef DEBUG
    std::cout << i_field << " " << "acqTypeAllowed = " << acqTypeAllowed
              << std::endl;
#endif
    coincidence_in_use_ = FiberTrackerField(r, ++i_field).toString();
#ifdef DEBUG
    std::cout << i_field << " " << "coincidenceInUse = " << coincidenceInUse
              << std::endl;
#endif
    counts_ = FiberTrackerField(r, ++i_field).toInt();
#ifdef DEBUG
    std::cout << i_field << " " << "counts = " << counts << std::endl;
#endif
    counts_records_ = FiberTrackerField(r, ++i_field).toLong();
#ifdef DEBUG
    std::cout << i_field << " " << "countsRecords = " << countsRecords
              << std::endl;
#endif
    counts_records_with_zero_events_ = FiberTrackerField(r, ++i_field).toLong();
#ifdef DEBUG
    std::cout << i_field << " "
              << "countsRecordsWithZeroEvents = " << countsRecordsWithZeroEvents
              << std::endl;
#endif
    counts_trigs_ = FiberTrackerField(r, ++i_field).toLong();
#ifdef DEBUG
    std::cout << i_field << " " << "countsTrigs = " << countsTrigs << std::endl;
#endif
    cycle_name_ = FiberTrackerField(r, ++i_field).toString();
#ifdef DEBUG
    std::cout << i_field << " " << "cycleName = " << cycleName << std::endl;
#endif
    cycle_stamp_ = FiberTrackerField(r, ++i_field).toLong();
#ifdef DEBUG
    std::cout << i_field << " " << "cycleStamp = " << cycleStamp << std::endl;
#endif
    equipment_name_ = FiberTrackerField(r, ++i_field).toString();
#ifdef DEBUG
    std::cout << i_field << " " << "equipmentName = " << equipmentName
              << std::endl;
#endif
    event_selection_acq_ = FiberTrackerField(r, ++i_field).toInt();
#ifdef DEBUG
    std::cout << i_field << " " << "eventSelectionAcq = " << eventSelectionAcq
              << std::endl;
#endif
    FiberTrackerField events_data_field(r, ++i_field);
    i_event_ = -1;
    events_data_.clear();
    events_data_.reserve(events_data_field.value().size() / 10);
    for (std::size_t i_word{0}; i_word < events_data_field.value().size();
         i_word += 10) {
      events_data_.emplace_back(events_data_field.value(), i_word);
    }
#ifdef DEBUG
    std::cout << i_field << " " << "eventsData (size = " << eventsData.size()
              << ")" << std::endl;
#endif
    mean_s_new_ = FiberTrackerField(r, ++i_field).toDouble();
#ifdef DEBUG
    std::cout << i_field << " " << "meanSNew = " << meanSNew << std::endl;
#endif
    message_ = FiberTrackerField(r, ++i_field).toString();
#ifdef DEBUG
    std::cout << i_field << " " << "message = " << message << std::endl;
#endif
    FiberTrackerField profile_field(r, ++i_field);
    profile_.clear();
    profile_.reserve(profile_field.value().size() / 2);
    for (std::size_t i_word{0}; i_word < profile_field.value().size();
         i_word += 2) {
      profile_.push_back(profile_field.toDouble(i_word));
    }
#ifdef DEBUG
    std::cout << i_field << " " << "profile size " << profile.size()
              << std::endl;
#endif
    // fields 18 and 19 are skipped
    i_field += 2;
    FiberTrackerField profile_stand_alone_field(r, ++i_field);
    profile_stand_alone_.clear();
    profile_stand_alone_.reserve(profile_stand_alone_field.value().size() / 2);
    for (std::size_t i_word{0};
         i_word < profile_stand_alone_field.value().size(); i_word += 2) {
      profile_stand_alone_.push_back(
          profile_stand_alone_field.toDouble(i_word));
    }
#ifdef DEBUG
    std::cout << i_field << " " << "profileStandAlone size "
              << profileStandAlone.size() << std::endl;
#endif
    time_first_event_ = FiberTrackerField(r, ++i_field).toString();
#ifdef DEBUG
    std::cout << i_field << " " << "timeFirstEvent = " << timeFirstEvent
              << std::endl;
#endif
    time_first_trigger_ = FiberTrackerField(r, ++i_field).toString();
#ifdef DEBUG
    std::cout << i_field << " " << "timeFirstTrigger = " << timeFirstTrigger
              << std::endl;
#endif
    time_last_event_ = FiberTrackerField(r, ++i_field).toString();
#ifdef DEBUG
    std::cout << i_field << " " << "timeLastEvent = " << timeLastEvent
              << std::endl;
#endif
    time_last_trigger_ = FiberTrackerField(r, ++i_field).toString();
#ifdef DEBUG
    std::cout << i_field << " " << "timeLastTrigger = " << timeLastTrigger
              << std::endl;
#endif
    trigger_ = FiberTrackerField(r, ++i_field).toInt();
#ifdef DEBUG
    std::cout << i_field << " " << "trigger = " << trigger << std::endl;
#endif
    trigger_offset_acq_ = FiberTrackerField(r, ++i_field).toInt();
#ifdef DEBUG
    std::cout << i_field << " " << "triggerOffsetAcq = " << triggerOffsetAcq
              << std::endl;
#endif
    trigger_selection_acq_ = FiberTrackerField(r, ++i_field).toInt();
#ifdef DEBUG
    std::cout << i_field << " "
              << "triggerSelectionAcq = " << triggerSelectionAcq << std::endl;
#endif
    return r;
  }
};

std::ostream& operator<<(std::ostream& os, const FiberTrackerBinaryPacket& p) {
  return (os << "FiberTracker Packet {" << p.acq_stamp_ << "}");
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
  TTree *tree_, *spill_tree_;
};

void FiberTrackerRawDecoder::configure(framework::config::Parameters& ps) {
  input_file_ = ps.get<std::string>("input_file");
  output_name_ = ps.get<std::string>("output_name");
  ntuplize_ = ps.get<bool>("ntuplize");

  file_reader_.open(input_file_);
}

void FiberTrackerRawDecoder::onProcessStart() {
  if (ntuplize_) {
    getHistoDirectory();
    tree_ = new TTree("raw", "Flattened and decoded raw FiberTracker data");
    tree_->Branch("TriggerTSLSB", &ft_event_.trigger_timestamp_lsb_);
    tree_->Branch("TriggerTSMSB", &ft_event_.trigger_timestamp_msb_);
    tree_->Branch("EventTSLSB", &ft_event_.event_timestamp_lsb_);
    tree_->Branch("EventTSMSB", &ft_event_.event_timestamp_msb_);
    tree_->Branch("ChannelHits", &ft_event_.channel_hits_);

    spill_tree_ = new TTree("spill", "Spill Meta-Data from FiberTrackerDAQ");
    spill_tree_->Branch("acqMode", &spill_packet_.acq_mode_);
    spill_tree_->Branch("acqStamp", &spill_packet_.acq_stamp_);
    spill_tree_->Branch("acqType", &spill_packet_.acq_type_);
    spill_tree_->Branch("acqTypeAllowed", &spill_packet_.acq_type_allowed_);
    spill_tree_->Branch("counts", &spill_packet_.counts_);
    spill_tree_->Branch("countsRecords", &spill_packet_.counts_records_);
    spill_tree_->Branch("countsRecordsWithZeroEvents",
                        &spill_packet_.counts_records_with_zero_events_);
    spill_tree_->Branch("countsTrigs", &spill_packet_.counts_trigs_);
    spill_tree_->Branch("cycleStamp", &spill_packet_.cycle_stamp_);
    spill_tree_->Branch("eventSelectionAcq",
                        &spill_packet_.event_selection_acq_);
    spill_tree_->Branch("meanSNew", &spill_packet_.mean_s_new_);
    spill_tree_->Branch("profile", &spill_packet_.profile_);
    spill_tree_->Branch("profileStandAlone",
                        &spill_packet_.profile_stand_alone_);
    spill_tree_->Branch("trigger", &spill_packet_.trigger_);
    spill_tree_->Branch("triggerOffsetAcq", &spill_packet_.trigger_offset_acq_);
    spill_tree_->Branch("triggerSelectionAcq",
                        &spill_packet_.trigger_selection_acq_);
  }
}

void FiberTrackerRawDecoder::produce(framework::Event& event) {
  // only add and fill when file able to readout packet
  if (not spill_packet_.next(ft_event_)) {
    // no more events in this spill
    if (file_reader_ >> spill_packet_) {
      spill_tree_->Fill();
      // next spill loaded, pop its first event
      spill_packet_.next(ft_event_);
    } else {
#ifdef DEBUG
      std::cout << "no more events" << std::endl;
#endif
      return;
    }
  }
  tree_->Fill();

  event.add(output_name_ + "TriggerTSLSB", ft_event_.trigger_timestamp_lsb_);
  event.add(output_name_ + "TriggerTSMSB", ft_event_.trigger_timestamp_msb_);
  event.add(output_name_ + "EventTSLSB", ft_event_.event_timestamp_lsb_);
  event.add(output_name_ + "EventTSMSB", ft_event_.event_timestamp_msb_);
  event.add(output_name_ + "Hits", ft_event_.channel_hits_);
  return;
}  // produce

}  // namespace packing

DECLARE_PRODUCER(packing::FiberTrackerRawDecoder);

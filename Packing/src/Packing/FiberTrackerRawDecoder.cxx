
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
double to_double(uint64_t i) {
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

std::string to_string(const std::vector<uint32_t>& field) {
  std::string str;
  str.resize(field.size()-1); // remove header
  for (int i{0}; i < str.size(); i++) {
    str[i] = (char)field[i+1];
  }
  return str;
}

long int to_long(const std::vector<uint32_t>& field) {
  return ((uint64_t)field.at(2) << 32) | (uint64_t)field.at(1);
}

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
  std::vector<int> eventsData;
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
  utility::Reader& read(utility::Reader& r) {
    uint32_t eventlen, fieldlen, fieldheader, i_word{0};
    r >> eventlen;
    while (i_word < eventlen) {
      r >> fieldlen;
      i_word += fieldlen+2; // move forward by field length and two field headers
      std::vector<uint32_t> k;
      r.read(k, fieldlen+1);
      if( k[0] == 1 ){
        acqMode = k[1];
      } else if( k[0] == 2 ){
        acqStamp = to_long(k);
      } else if( k[0] == 3){
        acqType = k[1];
      } else if( k[0] == 4){
        acqTypeAllowed = k[1];
      } else if( k[0] == 5){
        coincidenceInUse = to_string(k);
      } else if( k[0] == 6 ){
        counts = k[1];
      } else if( k[0] == 7 ){
        countsRecords = to_long(k);
      } else if( k[0] == 8 ){
        countsRecordsWithZeroEvents = to_long(k);
      } else if( k[0] == 9 ){
        countsTrigs = to_long(k);
      } else if( k[0] == 10){
        cycleName = to_string(k);
      } else if( k[0] == 11 ){
        cycleStamp = to_long(k);
      } else if( k[0] == 12){
        equipmentName = to_string(k);
      } else if( k[0] == 13){
        eventSelectionAcq = k[1];
      } else if( k[0] == 14 ){
        eventsData.clear();
        eventsData.reserve(k.size()-1);
        for(int i = 1; i < k.size(); i++){
          eventsData.push_back(k.at(i));
        }
      } else if( k[0] == 15 ){
        meanSNew = to_double(to_long(k));
      } else if( k[0] == 16){
        message = to_string(k);
      } else if( k[0] == 17){
        profile.clear();
        profile.reserve(k.size()/2);
        //std::cout << "profile" << std::endl;
        for(int i = 1; i < k.size(); i+=2){
          uint64_t p = ((uint64_t)k.at(i+1) << 32) | (uint64_t)k.at(i);
          profile[(i+1)/2-1] = to_double(p);
        }
      } else if( k[0] == 20){
        profileStandAlone.clear();
        profileStandAlone.reserve(k.size()/2);
        for(int i = 1; i < k.size(); i+=2){
          uint64_t p = ((uint64_t)k.at(i+1) << 32) | (uint64_t)k.at(i);
          profileStandAlone[(i+1)/2-1] = to_double(p);
        }
      } else if( k[0] == 21){
        timeFirstEvent = to_string(k);
      } else if( k[0] == 22){
        timeFirstTrigger = to_string(k);
      } else if( k[0] == 23){
        timeLastEvent = to_string(k);
      } else if( k[0] == 24){
        timeLastTrigger = to_string(k);
      } else if( k[0] == 25){
        trigger = k[1];
      } else if( k[0] == 26){
        triggerOffsetAcq = k[1];
      } else if( k[0] == 27){
        triggerSelectionAcq = k[1];
      }
    }

    return r;
  } 
  void add(framework::Event& event, const std::string& name) {
    event.add(name+"acqMode",acqMode);
    event.add(name+"acqStamp",acqStamp);
    event.add(name+"acqType",acqType);
    event.add(name+"acqTypeAllowed",acqTypeAllowed);
    //event.add(name+"coincidenceInUse",coincidenceInUse);
    event.add(name+"counts",counts);
    event.add(name+"countsRecords",countsRecords);
    event.add(name+"countsRecordsWithZeroEvents",countsRecordsWithZeroEvents);
    event.add(name+"countsTrigs",countsTrigs);
    //event.add(name+"cycleName",cycleName);
    event.add(name+"cycleStamp",cycleStamp);
    //event.add(name+"equipmentName",equipmentName);
    event.add(name+"eventSelectionAcq",eventSelectionAcq);
    event.add(name+"eventsData",eventsData);
    event.add(name+"meanSNew",meanSNew);
    //event.add(name+"message",message);
    event.add(name+"profile",profile);
    event.add(name+"profileStandAlone",profileStandAlone);
    //event.add(name+"timeFirstEvent",timeFirstEvent);
    //event.add(name+"timeFirstTrigger",timeFirstTrigger);
    //event.add(name+"timeLastEvent",timeLastEvent);
    //event.add(name+"timeLastTrigger",timeLastTrigger);
    event.add(name+"trigger",trigger);
    event.add(name+"triggerOffsetAcq",triggerOffsetAcq);
    event.add(name+"triggerSelectionAcq",triggerSelectionAcq);
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
  FiberTrackerBinaryPacket p;
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
    tree_->Branch("acqMode",&p.acqMode);
    tree_->Branch("acqStamp",&p.acqStamp);
    tree_->Branch("acqType",&p.acqType);
    tree_->Branch("acqTypeAllowed",&p.acqTypeAllowed);
    tree_->Branch("coincidenceInUse",&p.coincidenceInUse);
    tree_->Branch("counts",&p.counts);
    tree_->Branch("countsRecords",&p.countsRecords);
    tree_->Branch("countsRecordsWithZeroEvents",&p.countsRecordsWithZeroEvents);
    tree_->Branch("countsTrigs",&p.countsTrigs);
    tree_->Branch("cycleName",&p.cycleName);
    tree_->Branch("cycleStamp",&p.cycleStamp);
    tree_->Branch("equipmentName",&p.equipmentName);
    tree_->Branch("eventSelectionAcq",&p.eventSelectionAcq);
    tree_->Branch("eventsData",&p.eventsData);
    tree_->Branch("meanSNew",&p.meanSNew);
    tree_->Branch("message",&p.message);
    tree_->Branch("profile",&p.profile);
    tree_->Branch("profileStandAlone",&p.profileStandAlone);
    tree_->Branch("timeFirstEvent",&p.timeFirstEvent);
    tree_->Branch("timeFirstTrigger",&p.timeFirstTrigger);
    tree_->Branch("timeLastEvent",&p.timeLastEvent);
    tree_->Branch("timeLastTrigger",&p.timeLastTrigger);
    tree_->Branch("trigger",&p.trigger);
    tree_->Branch("triggerOffsetAcq",&p.triggerOffsetAcq);
    tree_->Branch("triggerSelectionAcq",&p.triggerSelectionAcq);
  }
}

void FiberTrackerRawDecoder::produce(framework::Event& event) {
  // only add and fill when file able to readout packet
  if (file_reader_ >> p) {
    p.add(event,output_name_);
    tree_->Fill();
#ifdef DEBUG
    std::cout << p << std::endl;
#endif
  }
#ifdef DEBUG
  else {
    std::cout << "no more events" << std::endl;
  }
#endif
  return;
}  // produce

}  // namespace packing

DECLARE_PRODUCER_NS(packing, FiberTrackerRawDecoder);

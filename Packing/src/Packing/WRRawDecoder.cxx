
#include <bitset>
#include <iomanip>
#include <optional>

#include "Framework/EventProcessor.h"
#include "Packing/Utility/Mask.h"
#include "Packing/Utility/Reader.h"

// un comment for WRRawDecoder-specific debug printouts to std::cout
//#define DEBUG

namespace packing {

struct WRBinaryPacket {
  int runNumber;
  int WRCounter;
  int channel;
  int seq_id;
  int sec;
  int coarse;
  int frac;
  utility::Reader& read(utility::Reader& r) {
    return r >> runNumber >> WRCounter >> channel >> seq_id >> sec >> coarse >> frac;
  } 
  void add(framework::Event& event, const std::string& name) {
    event.add(name+"RunNumber",runNumber);
    event.add(name+"Counter",WRCounter);
    event.add(name+"Channel",channel);
    event.add(name+"SeqId",seq_id);
    event.add(name+"Sec",sec);
    event.add(name+"Coarse",coarse);
    event.add(name+"Frac",frac);
  }
};

std::ostream& operator<<(std::ostream& os, const WRBinaryPacket& p) {
  return (os << "WR Packet {"
        << "run: " << p.runNumber
        << ", counter: " << p.WRCounter
        << ", channel: " << p.channel
        << ", seq_id: " << p.seq_id
        << ", sec: " << p.sec
        << ", coarse: " << p.coarse
        << ", frac: " << p.frac
        << "}"
        );
}

/**
 * @class WRRawDecoder
 */
class WRRawDecoder : public framework::Producer {
 public:
  WRRawDecoder(const std::string& name, framework::Process& process)
      : framework::Producer(name, process) {}
  virtual ~WRRawDecoder() = default;
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
  WRBinaryPacket p;
  /// ntuple tree
  TTree* tree_;
};

void WRRawDecoder::configure(framework::config::Parameters& ps) {
  input_file_ = ps.getParameter<std::string>("input_file");
  output_name_ = ps.getParameter<std::string>("output_name");
  ntuplize_ = ps.getParameter<bool>("ntuplize");

  file_reader_.open(input_file_);
}

void WRRawDecoder::onProcessStart() {
  if (ntuplize_) {
    getHistoDirectory();
    tree_ = new TTree("wrraw","Flattened and decoded raw WR data");
    tree_->Branch("run",&p.runNumber);
    tree_->Branch("counter",&p.WRCounter);
    tree_->Branch("channel",&p.channel);
    tree_->Branch("seq_id",&p.seq_id);
    tree_->Branch("sec",&p.sec);
    tree_->Branch("coarse",&p.coarse);
    tree_->Branch("frac",&p.frac);
  }
}

void WRRawDecoder::produce(framework::Event& event) {
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

DECLARE_PRODUCER_NS(packing, WRRawDecoder);


#include <bitset>
#include <iomanip>
#include <optional>

#include "Framework/EventProcessor.h"
#include "Packing/Utility/Mask.h"
#include "Packing/Utility/Reader.h"

// un comment for WRRawDecoder-specific debug printouts to std::cout
// #define DEBUG

namespace packing {

struct WRBinaryPacket {
  int run_number_;
  int wr_counter_;
  int channel_;
  int seq_id_;
  int sec_;
  int coarse_;
  int frac_;
  utility::Reader& read(utility::Reader& r) {
    return r >> run_number_ >> wr_counter_ >> channel_ >> seq_id_ >> sec_ >>
           coarse_ >> frac_;
  }
  void add(framework::Event& event, const std::string& name) {
    event.add(name + "RunNumber", run_number_);
    event.add(name + "Counter", wr_counter_);
    event.add(name + "Channel", channel_);
    event.add(name + "SeqId", seq_id_);
    event.add(name + "Sec", sec_);
    event.add(name + "Coarse", coarse_);
    event.add(name + "Frac", frac_);
  }
};

std::ostream& operator<<(std::ostream& os, const WRBinaryPacket& p) {
  return (os << "WR Packet {" << "run: " << p.run_number_
             << ", counter: " << p.wr_counter_ << ", channel: " << p.channel_
             << ", seq_id: " << p.seq_id_ << ", sec: " << p.sec_
             << ", coarse: " << p.coarse_ << ", frac: " << p.frac_ << "}");
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
  WRBinaryPacket p_;
  /// ntuple tree
  TTree* tree_;
};

void WRRawDecoder::configure(framework::config::Parameters& ps) {
  input_file_ = ps.get<std::string>("input_file");
  output_name_ = ps.get<std::string>("output_name");
  ntuplize_ = ps.get<bool>("ntuplize");

  file_reader_.open(input_file_);
}

void WRRawDecoder::onProcessStart() {
  if (ntuplize_) {
    getHistoDirectory();
    tree_ = new TTree("wrraw", "Flattened and decoded raw WR data");
    tree_->Branch("run", &p_.run_number_);
    tree_->Branch("counter", &p_.wr_counter_);
    tree_->Branch("channel", &p_.channel_);
    tree_->Branch("seq_id", &p_.seq_id_);
    tree_->Branch("sec", &p_.sec_);
    tree_->Branch("coarse", &p_.coarse_);
    tree_->Branch("frac", &p_.frac_);
  }
}

void WRRawDecoder::produce(framework::Event& event) {
  // only add and fill when file able to readout packet
  if (file_reader_ >> p_) {
    p_.add(event, output_name_);
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

DECLARE_PRODUCER(packing::WRRawDecoder);

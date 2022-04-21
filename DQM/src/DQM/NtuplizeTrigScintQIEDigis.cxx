
#include "Framework/EventProcessor.h"
#include "TrigScint/Event/TrigScintQIEDigis.h"

namespace dqm {

class NtuplizeTrigScintQIEDigis : public framework::Analyzer {
  std::string input_name_, input_pass_;

  int chan_id_, elec_id_, ldmxsw_event_;
  uint32_t time_since_spill_;
  std::vector<int> adc_, tdc_, cid_;

  TTree* flat_tree_;

 public:
  NtuplizeTrigScintQIEDigis(std::string const& n, framework::Process& p)
      : framework::Analyzer(n, p) {}
  ~NtuplizeTrigScintQIEDigis() {}

  void configure(framework::config::Parameters& ps) final override {
    input_name_ = ps.getParameter<std::string>("input_name");
    input_pass_ = ps.getParameter<std::string>("input_pass");
  }

  void onProcessStart() final override;
  void analyze(const framework::Event& event) final override;
};

void NtuplizeTrigScintQIEDigis::onProcessStart() {
  getHistoDirectory();
  // cleaned up when histogram file is closed
  flat_tree_ = new TTree("digis", "Ntuplized TS Digis");

  flat_tree_->Branch("time_since_spill", &time_since_spill_);
  flat_tree_->Branch("ldmxsw_event", &ldmxsw_event_);
  flat_tree_->Branch("chan_id", &chan_id_);
  flat_tree_->Branch("elec_id", &elec_id_);
  flat_tree_->Branch("adc", &adc_);
  flat_tree_->Branch("tdc", &tdc_);
  flat_tree_->Branch("cid", &cid_);
}

void NtuplizeTrigScintQIEDigis::analyze(const framework::Event& event) {
  const auto& digis{event.getCollection<trigscint::TrigScintQIEDigis>(input_name_,input_pass_)};
  ldmxsw_event_ = event.getEventNumber();
  for (const auto& digi : digis) {
    time_since_spill_ = digi.getTimeSinceSpill();
    chan_id_ = digi.getChanID();
    elec_id_ = digi.getElecID();
    adc_ = digi.getADC();
    tdc_ = digi.getTDC();
    cid_ = digi.getCID();
    flat_tree_->Fill();
  }
}

}  // namespace dqm

DECLARE_ANALYZER_NS(dqm, NtuplizeTrigScintQIEDigis);

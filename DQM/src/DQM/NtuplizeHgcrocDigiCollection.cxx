
#include "DetDescr/HcalElectronicsID.h"
#include "Framework/EventProcessor.h"
#include "Recon/Event/HgcrocDigiCollection.h"
#include "Conditions/SimpleTableCondition.h"

namespace dqm {

class NtuplizeHgcrocDigiCollection : public framework::Analyzer {
  std::string input_name_, input_pass_, pedestal_table_;
  int raw_id_, fpga_, link_, channel_, index_, adc_, raw_adc_, i_sample_, event_;
  TTree* flat_tree_;

 public:
  NtuplizeHgcrocDigiCollection(std::string const& n, framework::Process& p)
      : framework::Analyzer(n, p) {}
  ~NtuplizeHgcrocDigiCollection() {}

  void configure(framework::config::Parameters& ps) final override {
    input_name_ = ps.getParameter<std::string>("input_name");
    input_pass_ = ps.getParameter<std::string>("input_pass");
    pedestal_table_ = ps.getParameter<std::string>("pedestal_table");
  }

  void onProcessStart() final override {
    getHistoDirectory();
    // cleaned up when histogram file is closed
    flat_tree_ = new TTree("adc", "adc");

    flat_tree_->Branch("fpga", &fpga_);
    flat_tree_->Branch("raw_id", &raw_id_);
    flat_tree_->Branch("link", &link_);
    flat_tree_->Branch("channel", &channel_);
    flat_tree_->Branch("index", &index_);
    flat_tree_->Branch("adc", &adc_);
    flat_tree_->Branch("raw_adc", &raw_adc_);
    flat_tree_->Branch("i_sample", &i_sample_);
    flat_tree_->Branch("event", &event_);
  }

  void analyze(const framework::Event& event) final override;
};

void NtuplizeHgcrocDigiCollection::analyze(const framework::Event& event) {
  // get the reconstruction parameters 
  auto pedestal_table{getCondition<conditions::IntegerTableCondition>(pedestal_table_)};

  event_ = event.getEventNumber();

  auto const& digis{
      event.getObject<ldmx::HgcrocDigiCollection>(input_name_, input_pass_)};
  for (std::size_t i_digi{0}; i_digi < digis.size(); i_digi++) {
    auto d{digis.getDigi(i_digi)};
    ldmx::HcalElectronicsID eid(d.id());
    // undo hardcoded shifts in hcal raw decoder
    fpga_ = eid.fiber() + 5;
    link_ = eid.elink(); // leave shifted to align with link number
    channel_ = eid.channel() + 1;
    index_ = eid.index();
    raw_id_ = static_cast<int>(d.id());

    for (i_sample_ = 0; i_sample_ < digis.getNumSamplesPerDigi(); i_sample_++) {
      int adc_t = d.at(i_sample_).adc_t();
      raw_adc_ = adc_t;
      adc_ =  adc_t - pedestal_table.get(d.id(), 0);
      flat_tree_->Fill();
    }
  }
}

}  // namespace dqm

DECLARE_ANALYZER_NS(dqm, NtuplizeHgcrocDigiCollection);

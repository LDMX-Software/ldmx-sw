
#include "DetDescr/HcalElectronicsID.h"
#include "DetDescr/HcalDigiID.h"
#include "Framework/EventProcessor.h"
#include "Recon/Event/HgcrocDigiCollection.h"
#include "Conditions/SimpleTableCondition.h"

namespace dqm {

class NtuplizeHgcrocDigiCollection : public framework::Analyzer {
  std::string input_name_, input_pass_, pedestal_table_;
  int raw_id_, adc_, raw_adc_, tot_, toa_, i_sample_, event_;
  int fpga_, link_, channel_, index_;
  int section_, layer_, strip_, end_;
  bool tot_prog_, tot_comp_;
  bool using_eid_;
  TTree* flat_tree_;

 public:
  NtuplizeHgcrocDigiCollection(std::string const& n, framework::Process& p)
      : framework::Analyzer(n, p) {}
  ~NtuplizeHgcrocDigiCollection() {}

  void configure(framework::config::Parameters& ps) final override {
    input_name_ = ps.getParameter<std::string>("input_name");
    input_pass_ = ps.getParameter<std::string>("input_pass");
    pedestal_table_ = ps.getParameter<std::string>("pedestal_table");
    using_eid_ = ps.getParameter<bool>("using_eid");
  }

  void onProcessStart() final override {
    getHistoDirectory();
    // cleaned up when histogram file is closed
    flat_tree_ = new TTree("hgcroc", "Ntuplized HGC ROC Digi Collection");

    flat_tree_->Branch("raw_id", &raw_id_);
    flat_tree_->Branch("adc", &adc_);
    flat_tree_->Branch("tot", &tot_);
    flat_tree_->Branch("toa", &toa_);
    flat_tree_->Branch("raw_adc", &raw_adc_);
    flat_tree_->Branch("i_sample", &i_sample_);
    flat_tree_->Branch("event", &event_);
    flat_tree_->Branch("tot_prog", &tot_prog_);
    flat_tree_->Branch("tot_comp", &tot_comp_);
    if (using_eid_) {
      flat_tree_->Branch("fpga", &fpga_);
      flat_tree_->Branch("link", &link_);
      flat_tree_->Branch("channel", &channel_);
      flat_tree_->Branch("index", &index_);
    } else {
      flat_tree_->Branch("section", &section_);
      flat_tree_->Branch("layer", &layer_);
      flat_tree_->Branch("strip", &strip_);
      flat_tree_->Branch("end", &end_);
    }
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
    raw_id_ = static_cast<int>(d.id());
    if (using_eid_) {
      ldmx::HcalElectronicsID eid(d.id());
      fpga_ = eid.fiber();
      link_ = eid.elink();
      channel_ = eid.channel() + 1; // undo shift of channel numbers down to zero-based
      index_ = eid.index();
    } else {
      ldmx::HcalDigiID detid(d.id());
      section_ = detid.section();
      layer_ = detid.layer();
      strip_ = detid.strip();
      end_ = detid.end();
    }

    for (i_sample_ = 0; i_sample_ < digis.getNumSamplesPerDigi(); i_sample_++) {
      tot_prog_ = d.at(i_sample_).isTOTinProgress();
      tot_comp_ = d.at(i_sample_).isTOTComplete();
      tot_ = d.at(i_sample_).tot();
      toa_ = d.at(i_sample_).toa();
      int adc_t = d.at(i_sample_).adc_t();
      raw_adc_ = adc_t;
      adc_ =  adc_t - pedestal_table.get(d.id(), 0);
      flat_tree_->Fill();
    }
  }
}

}  // namespace dqm

DECLARE_ANALYZER_NS(dqm, NtuplizeHgcrocDigiCollection);

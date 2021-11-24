
#include "DetDescr/HcalElectronicsID.h"
#include "Framework/EventProcessor.h"
#include "Recon/Event/HgcrocDigiCollection.h"

namespace dqm {
namespace umn {

class TestHgcRoc : public framework::Analyzer {
  std::string input_name_, input_pass_;
  int fpga_, link_, channel_, index_, adc_;
  TTree* flat_tree_;

 public:
  TestHgcRoc(std::string const& n, framework::Process& p)
      : framework::Analyzer(n, p) {}
  ~TestHgcRoc() {}

  void configure(framework::config::Parameters& ps) final override {
    input_name_ = ps.getParameter<std::string>("input_name");
    input_pass_ = ps.getParameter<std::string>("input_pass");
  }

  void onProcessStart() final override {
    getHistoDirectory();
    // cleaned up when histogram file is closed
    flat_tree_ = new TTree("adc", "adc");

    flat_tree_->Branch("fpga", &fpga_);
    flat_tree_->Branch("link", &link_);
    flat_tree_->Branch("channel", &channel_);
    flat_tree_->Branch("index", &index_);
    flat_tree_->Branch("adc", &adc_);
  }

  void analyze(const framework::Event& event) final override {
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
      adc_ = d.soi().adc_t();

      flat_tree_->Fill();
    }
  }
};

}  // namespace umn
}  // namespace dqm

DECLARE_ANALYZER_NS(dqm::umn, TestHgcRoc);

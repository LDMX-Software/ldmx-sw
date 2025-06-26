#include "DQM/HgcrocPulseTruth.h"

#include "Recon/Event/HgcrocDigiCollection.h"
#include "Recon/Event/HgcrocPulseTruth.h"

namespace dqm {

void HgcrocPulseTruth::configure(framework::config::Parameters& ps) {
  input_digi_name_ = ps.getParameter<std::string>("input_digi_name");
  input_digi_pass_ = ps.getParameter<std::string>("input_digi_pass");
  input_truth_name_ = ps.getParameter<std::string>("input_truth_name");
  input_truth_pass_ = ps.getParameter<std::string>("input_truth_pass");
}

void HgcrocPulseTruth::onProcessStart() {
  getHistoDirectory();

  vpeak_sumADC_graph_ = new TGraph();
  vpeak_sumADC_graph_->SetMarkerSize(1);
  vpeak_sumADC_graph_->SetMarkerStyle(5);
  vpeak_sumADC_graph_->SetLineWidth(0.0);
  vpeak_sumADC_graph_->SetTitle(
      "All channels;Truth pulse peak voltage;Digi sum of ADC");
  vpeak_sumADC_graph_->SetName("vpeak_sumADC_all_ch");

  vpeak_TOT_graph_ = new TGraph();
  vpeak_TOT_graph_->SetMarkerSize(1);
  vpeak_TOT_graph_->SetMarkerStyle(5);
  vpeak_TOT_graph_->SetLineWidth(0.0);
  vpeak_TOT_graph_->SetTitle("All channels;Truth pulse peak voltage;Digi TOT");
  vpeak_TOT_graph_->SetName("vpeak_TOT_all_ch");
}

void HgcrocPulseTruth::onProcessEnd() {
  getHistoDirectory();
  vpeak_sumADC_graph_->Write();
  vpeak_TOT_graph_->Write();
}

void HgcrocPulseTruth::analyze(const framework::Event& event) {
  auto digis{event.getObject<ldmx::HgcrocDigiCollection>(input_digi_name_,
                                                         input_digi_pass_)};
  auto truths{event.getObject<ldmx::HgcrocPulseTruthCollection>(
      input_truth_name_, input_truth_pass_)};

  for (auto const& t : truths) {
    unsigned int id = t.getID();

    double vpeak = t.getMax();

    for (auto const& d : digis) {
      if (d.id() == id) {
        if (d.isADC()) {
          int sumADC = 0;
          for (int i = 0; i < d.size(); i++) {
            sumADC += d.at(i).adc_t();
          }
          vpeak_sumADC_graph_->AddPoint(vpeak, sumADC);
        } else {
          vpeak_TOT_graph_->AddPoint(vpeak, d.tot());
        }
        break;
      }
    }
  }
}

}  // namespace dqm

DECLARE_ANALYZER(dqm::HgcrocPulseTruth)

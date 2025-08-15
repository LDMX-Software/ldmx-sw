#include "DQM/HgcrocPulseTruthAnalyzer.h"

#include "Recon/Event/HgcrocDigiCollection.h"
#include "Recon/Event/HgcrocPulseTruth.h"

namespace dqm {

void HgcrocPulseTruthAnalyzer::configure(framework::config::Parameters& ps) {
  input_digi_name_ = ps.getParameter<std::string>("input_digi_name");
  input_digi_pass_ = ps.getParameter<std::string>("input_digi_pass");
  input_truth_name_ = ps.getParameter<std::string>("input_truth_name");
  input_truth_pass_ = ps.getParameter<std::string>("input_truth_pass");
}

void HgcrocPulseTruthAnalyzer::analyze(const framework::Event& event) {
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
          int sum_adc = 0;
          for (int i = 0; i < d.size(); i++) {
            sum_adc += d.at(i).adc_t();
          }
          histograms_.fill("vpeak_sumADC", vpeak, sum_adc);
        } else {
          histograms_.fill("vpeak_TOT", vpeak, d.tot());
        }
        break;
      }
    }
  }
}

}  // namespace dqm

DECLARE_ANALYZER(dqm::HgcrocPulseTruthAnalyzer)

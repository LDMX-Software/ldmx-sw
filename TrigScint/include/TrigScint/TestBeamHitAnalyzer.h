/**
 * @file TestBeamHitAnalyzer.h
 * @brief
 * @author
 */

#ifndef TRIGSCINT_TESTBEAMHITANALYZER_H
#define TRIGSCINT_TESTBEAMHITANALYZER_H

// LDMX Framework
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"  //Needed to declare processor
#include "TH1.h"
#include "TH2.h"
#include "TrigScint/Event/TestBeamHit.h"

namespace trigscint {

/**
 * @class TestBeamHitAnalyzer
 * @brief
 */
class TestBeamHitAnalyzer : public framework::Analyzer {
 public:
  TestBeamHitAnalyzer(
      const std::string& name,
      framework::Process& process);  // : framework::Analyzer(name, process) {}
  virtual ~TestBeamHitAnalyzer() = default;
  void configure(framework::config::Parameters& parameters) override;

  void analyze(const framework::Event& event) override;

  void onProcessStart() override;

  void onProcessEnd() override;

 private:
  std::vector<std::vector<TH1F*> > v_charge_vs_time_;

  // configurable parameters
  std::string input_col_;
  std::string input_pass_name_{""};
  std::vector<double> peds_;
  int start_sample_{0};

  // plotting stuff
  int n_ev_{200};
  int n_channels_{16};
  // int nTrkMax{100};
  TH2F* h_ev_disp_;
  TH2F* h_ev_disp_pe_;

  int fill_nb_{0};

  // match nev, nchan above
  TH1F* h_out_[200][16];

  TH1F* h_pe_[16];
  TH1F* h_p_ein_clusters_[16];
  TH2F* h_pe_vs_delta_[16];
  TH2F* h_delta_pe_vs_delta_[16];
  TH2F* h_p_emax_vs_delta_;
  TH2F* h_cross_talk_[16][16];
};
}  // namespace trigscint

#endif /* TRIGSCINT_TESTBEAMHITANALYZER_H */

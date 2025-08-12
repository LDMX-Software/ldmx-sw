/**
 * @file QualityFlagAnalyzer.h
 * @brief
 * @author
 */

#ifndef TRIGSCINT_QUALITYFLAGANALYZER_H
#define TRIGSCINT_QUALITYFLAGANALYZER_H

// LDMX Framework
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"  //Needed to declare processor
#include "TH1.h"
#include "TH2.h"
#include "TrigScint/Event/EventReadout.h"
#include "TrigScint/Event/TestBeamHit.h"

namespace trigscint {

/**
 * @class QualityFlagAnalyzer
 * @brief
 */
class QualityFlagAnalyzer : public framework::Analyzer {
 public:
  QualityFlagAnalyzer(
      const std::string& name,
      framework::Process& process);  // : framework::Analyzer(name, process) {}
  virtual ~QualityFlagAnalyzer() = default;
  void configure(framework::config::Parameters& parameters) override;

  void analyze(const framework::Event& event) override;

  void onProcessStart() override;

  void onProcessEnd() override;

 private:
  std::vector<std::vector<TH1F*> > v_charge_vs_time_;

  // configurable parameters
  std::string input_event_col_;  // full event stream input
  std::string input_event_pass_name_{""};
  std::string input_hit_col_;  // hit collection
  std::string input_hit_pass_name_{""};
  std::vector<double> peds_;
  std::vector<double> gain_;
  int start_sample_{0};

  // plotting stuff
  const int N_EV{200};
  static constexpr int N_CHANNELS{16};
  const int N_FLAGS{6};
  int pe_fill_nb_{0};

  // make sure to match constants above
  const int FLAGS[6] = {16, 8, 4,
                        2,  1, 0};  // this order just makes looping easier
  int n_ev_drawn_[6] = {
      0};  // keep a counter for each flag type to get good stats
  TH1F* h_out_[200][16];
  TH1F* h_out_pe_[200][16];
  TH1F* h_out_flag_[6][200][16];  // for 4 quality flags and 0 (no flag)
  TH1F* h_pe_[16];

  TH2F* h_td_cfire_chanvs_event_;
};
}  // namespace trigscint

#endif /* TRIGSCINT_QUALITYFLAGANALYZER_H */

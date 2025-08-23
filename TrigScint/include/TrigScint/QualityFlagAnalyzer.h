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
  int n_ev_{200};
  static constexpr int N_CHANNELS{16};
  int n_flags_{6};
  int pe_fill_nb_{0};

  // make sure to match constants above
  // this order just makes looping easier
  int flags_[6] = {16, 8, 4, 2, 1, 0};
  // keep a counter for each flag type to get good stats
  int n_ev_drawn_[6] = {0};

  TH1F* h_out_[200][16];
  TH1F* h_out_pe_[200][16];
  TH1F* h_out_flag_[6][200][16];  // for 4 quality flags and 0 (no flag)
  TH1F* h_pe_[16];

  TH2F* h_td_cfire_chan_vs_event_;
};
}  // namespace trigscint

#endif /* TRIGSCINT_QUALITYFLAGANALYZER_H */

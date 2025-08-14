/**
 * @file QIEAnalyzer.h
 * @brief
 * @author
 */

#ifndef TRIGSCINT_QIEANALYZER_H
#define TRIGSCINT_QIEANALYZER_H

// LDMX Framework
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"  //Needed to declare processor
#include "TH1.h"
#include "TH2.h"
#include "TrigScint/Event/EventReadout.h"

namespace trigscint {

/**
 * @class QIEAnalyzer
 * @brief
 */
class QIEAnalyzer : public framework::Analyzer {
 public:
  QIEAnalyzer(
      const std::string& name,
      framework::Process& process);  // : framework::Analyzer(name, process) {}
  virtual ~QIEAnalyzer() = default;
  void configure(framework::config::Parameters& parameters) override;

  void analyze(const framework::Event& event) override;

  void onProcessStart() override;

  void onProcessEnd() override;

  float convertToID(float yVal) { return (yVal + y_offset_) * y_to_id_factor_; }

 private:
  std::vector<std::vector<TH1F*> > v_charge_vs_time_;

  // configurable parameters
  std::string input_col_;
  std::string input_pass_name_{""};
  std::vector<double> peds_;
  std::vector<double> gain_;
  int start_sample_{0};

  // plotting stuff
  int n_ev_{200};
  int n_channels_{16};
  // int nTrkMax{100};

  // match nev, nchan above
  TH1F* h_out_[200][16];
  TH1F* h_pe_[16];
  TH2F* h_pe_vs_t_[16];
  TH2F* h_ped_subtracted_avg_q_vs_t_[16];
  TH2F* h_ped_subtracted_tot_q_vs_ped_[16];
  TH2F* h_ped_subtracted_tot_q_vs_n_[16];
  TH2F* h_tot_q_vs_ped_[16];
  TH2F* h_ped_subtracted_pe_vs_n_[16];
  TH2F* h_ped_subtracted_pe_vs_t_[16];
  TH2F* h_avg_q_vs_t_[16];

  TH2F* h_tdc_fire_chan_vs_event_;
  double y_offset_{35.};
  double y_to_id_factor_{50. / 80.};
};
}  // namespace trigscint

#endif /* TRIGSCINT_QIEANALYZER_H */

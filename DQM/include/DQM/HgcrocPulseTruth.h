#ifndef _DQM_HGCROCPULSETRUTH_H_
#define _DQM_HGCROCPULSETRUTH_H_

/*~~~~~~~~~~~~~~~*/
/*   Framework   */
/*~~~~~~~~~~~~~~~*/
#include "Framework/Configure/Parameters.h"
#include "Framework/Event.h"
#include "Framework/EventProcessor.h"
#include "TGraph.h"
#include "Tools/AnalysisUtils.h"

namespace dqm {

class HgcrocPulseTruth : public framework::Analyzer {
 public:
  HgcrocPulseTruth(const std::string& name, framework::Process& process)
      : framework::Analyzer(name, process) {}

  ~HgcrocPulseTruth() {}

  void configure(framework::config::Parameters& parameters) override;

  void onProcessStart() override;

  void onProcessEnd() override;

  void analyze(const framework::Event& event) override;

 private:
  std::string input_digi_name_;
  std::string input_digi_pass_;
  std::string input_truth_name_;
  std::string input_truth_pass_;
};

}  // namespace dqm

#endif

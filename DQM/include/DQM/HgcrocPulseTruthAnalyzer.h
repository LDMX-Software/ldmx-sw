#ifndef _DQM_HGCROCPULSETRUTHANALYZER_H_
#define _DQM_HGCROCPULSETRUTHANALYZER_H_

/*~~~~~~~~~~~~~~~*/
/*   Framework   */
/*~~~~~~~~~~~~~~~*/
#include "Framework/Configure/Parameters.h"
#include "Framework/Event.h"
#include "Framework/EventProcessor.h"
#include "TGraph.h"
#include "Tools/AnalysisUtils.h"

namespace dqm {

class HgcrocPulseTruthAnalyzer : public framework::Analyzer {
 public:
  HgcrocPulseTruthAnalyzer(const std::string& name, framework::Process& process)
      : framework::Analyzer(name, process){};

  void configure(framework::config::Parameters& parameters) override;

  void analyze(const framework::Event& event) override;

 private:
  std::string input_digi_name_;
  std::string input_digi_pass_;
  std::string input_truth_name_;
  std::string input_truth_pass_;
};

}  // namespace dqm

#endif

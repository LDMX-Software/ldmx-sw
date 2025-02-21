#ifndef RUNHEADERANALYZER_H
#define RUNHEADERANALYZER_H

#include "Framework/EventProcessor.h"
#include "Framework/EventHeader.h"
#include "Framework/Process.h"
#include "Framework/RunHeader.h"

namespace framework {

class RunHeaderAnalyzer : public framework::Analyzer {

 public:
  RunHeaderAnalyzer(const std::string& n, framework::Process& p)
      : framework::Analyzer(n, p) {}
  virtual ~RunHeaderAnalyzer() = default;

  //void configure(framework::config::Parameters& ps) override {}
  void analyze(const framework::Event& event) override;
  void onNewRun(const ldmx::RunHeader& rh) override;
};

}  // namespace framework

#endif /* RUNHEADERANALYZER_H */

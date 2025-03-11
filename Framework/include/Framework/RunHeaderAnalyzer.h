#ifndef RUNHEADERANALYZER_H
#define RUNHEADERANALYZER_H

#include "Framework/EventHeader.h"
#include "Framework/EventProcessor.h"
#include "Framework/Process.h"
#include "Framework/RunHeader.h"

namespace framework {

class RunHeaderAnalyzer : public framework::Analyzer {
 public:
  /**
   * Constructor
   */
  RunHeaderAnalyzer(const std::string& name, framework::Process& process)
      : framework::Analyzer(name, process) {}

  /**
   * Destructor
   */
  virtual ~RunHeaderAnalyzer() = default;

  /**
   * This is used to print out the run header on a new run
   */
  virtual void onNewRun(const ldmx::RunHeader& rh) override;

  /**
   * Not used but necessary for an analyzer
   */
  virtual void analyze(const framework::Event& event) override;
};

}  // namespace framework

#endif /* RUNHEADERANALYZER_H */

#include "Framework/RunHeaderAnalyzer.h"

namespace framework {

void RunHeaderAnalyzer::onNewRun(const ldmx::RunHeader& rh) { rh.Print(); }

void RunHeaderAnalyzer::analyze(const framework::Event& event) { return; }

}  // namespace framework

DECLARE_ANALYZER_NS(framework, RunHeaderAnalyzer);
#include "Framework/RunHeaderAnalyzer.h"

namespace framework {

void RunHeaderAnalyzer::onNewRun(const ldmx::RunHeader& rh) { rh.print(); }

void RunHeaderAnalyzer::analyze(const framework::Event& event) { return; }

}  // namespace framework

DECLARE_ANALYZER(framework::RunHeaderAnalyzer);

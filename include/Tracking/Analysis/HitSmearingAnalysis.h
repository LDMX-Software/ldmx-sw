#ifndef ANALYSIS_HITSMEARING_H
#define ANALYSIS_HITSMEARING_H

// LDMX Framework
#include "Framework/EventProcessor.h"  // Needed to declare processor

namespace tracking {
  namespace analysis {

    /**
     * @class HitSmearingAnalysis
     * @brief Produce validation plots for the hit smearing processor
     */
    class HitSmearingAnalysis : public framework::Analyzer {
    public:
      HitSmearingAnalysis(const std::string& name, framework::Process& process)
	: Analyzer(name, process) {}
      
      /// print each event
      virtual void analyze(const framework::Event& event);
      virtual void onProcessStart();
    };
    
  }  // namespace analysis
}//namespace tracking
  
#endif /* ANALYSIS_HITSMEARING_H */
  

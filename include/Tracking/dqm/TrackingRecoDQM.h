#pragma once

#include "Framework/Event.h"
#include "Framework/EventProcessor.h"
#include "Tracking/Event/Track.h"


namespace tracking::dqm {

  class TrackingRecoDQM : public framework::Analyzer {
 public:
    
 TrackingRecoDQM(const std::string& name, framework::Process& process)
     : framework::Analyzer(name, process){};

    /// Destructor
    ~TrackingRecoDQM() = default;

    void analyze(const framework::Event& event) final override;

    void TrackMonitoring(const std::vector<ldmx::Track>& tracks,
                         const std::string name);

    
  };
}

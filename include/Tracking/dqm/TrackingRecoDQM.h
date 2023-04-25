#pragma once

#include "Framework/Event.h"
#include "Framework/EventProcessor.h"
#include "Framework/Configure/Parameters.h"
#include "Tracking/Event/Track.h"
#include "Tracking/Event/TruthTrack.h"



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

    /**
     * Configure the analyzer using the given user specified parameters.
     *
     * @param parameters Set of parameters used to configure this analyzer.
     */
    
    void configure(framework::config::Parameters &parameters) override;
    

   private:
    
    std::string trackCollection_{"TruthTracks"};
    std::string truthCollection_{"TaggerTruthTracks"};
    std::string title_{"tagger_trk_"};
    bool doTruthComparison{false};

    std::shared_ptr<std::vector<ldmx::TruthTrack>> truthTrackCollection_{nullptr};
    
  };
}


#pragma once

#include "Framework/Event.h"
#include "Framework/EventProcessor.h"
#include "Framework/Configure/Parameters.h"
#include "Tracking/Event/Track.h"
#include "Tracking/Event/TruthTrack.h"
#include "SimCore/Event/SimTrackerHit.h"


namespace tracking::dqm {




enum PIDBins {
  kminus     = -4,
  antiproton,
  piminus,
  positron,  
  electron,   
  piplus,       
  proton,
  kplus
};

  class TrackingRecoDQM : public framework::Analyzer {
 public:
    
 TrackingRecoDQM(const std::string& name, framework::Process& process)
     : framework::Analyzer(name, process){};

    /// Destructor
    ~TrackingRecoDQM() = default;

    void analyze(const framework::Event& event) final override;

    void TrackMonitoring(const std::vector<ldmx::Track>& tracks,
                         const std::string title,
                         const bool& doDetail,
                         const bool& doTruth);
    


    void EfficiencyPlots(const std::vector<ldmx::Track>& tracks,
                         const std::string& title);
    
    /** Monitoring plots for tracks extrapolated to the ECAL Scoring plane.
     *  This aims 
     *  Tracks will be truth matched first to get the trackID. The hit with the trackID 
     *
     */

    void TrackEcalScoringPlaneMonitoring(const std::vector<ldmx::Track>& tracks);

    void TrackTargetScoringPlaneMonitoring(const std::vector<ldmx::Track>& tracks);
    /**
     * Configure the analyzer using the given user specified parameters.
     *
     * @param parameters Set of parameters used to configure this analyzer.
     */
    
    void configure(framework::config::Parameters &parameters) override;

    void onProcessEnd() override;

    void sortTracks(const std::vector<ldmx::Track>& tracks,
                    std::vector<ldmx::Track>& uniqueTracks,
                    std::vector<ldmx::Track>& duplicateTracks,
                    std::vector<ldmx::Track>& fakeTracks);
    

   private:
    
    std::string trackCollection_{"TruthTracks"};
    std::string truthCollection_{"TaggerTruthTracks"};
    std::string title_{"tagger_trk_"};
    double trackProb_cut_{0.5};
    std::string subdetector_{"Tagger"};
    bool doTruthComparison{false};
    bool debug_{false};

    // Truth Track collection
    std::shared_ptr<std::vector<ldmx::TruthTrack>> truthTrackCollection_{nullptr};

    // Ecal scoring plane hits
    std::shared_ptr<std::vector<ldmx::SimTrackerHit>> ecal_scoring_hits_{nullptr};
    
    // Target  scoring plane hits
    std::shared_ptr<std::vector<ldmx::SimTrackerHit>> target_scoring_hits_{nullptr};
    
    //If I have truth information, sort the tracks vector according to their trackID and truthProb
    std::vector<ldmx::Track> uniqueTracks;     // real tracks (truth_prob > cut), unique
    std::vector<ldmx::Track> duplicateTracks;  // real tracks (truth_prob > cut), duplicated
    std::vector<ldmx::Track> fakeTracks;       // fake tracks (truth_prob < cut)


    //PID mapping
    std::map<int,int> pidmap;
        


    
    
    
  };
}


#ifndef TRACKING_RECO_TRUTHSEEDPROCESSOR_H_
#define TRACKING_RECO_TRUTHSEEDPROCESSOR_H_

//--- Framework ---//
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"

#include "SimCore/Event/SimTrackerHit.h"

// --- Tracking --- //
#include "Tracking/Event/Track.h"
#include "Tracking/Sim/TrackingUtils.h"

// --- ACTS --- //
#include "Acts/Definitions/Algebra.hpp"
#include "Acts/Definitions/TrackParametrization.hpp"
#include "Acts/EventData/TrackParameters.hpp"
#include "Acts/EventData/detail/TransformationFreeToBound.hpp"
#include "Acts/Surfaces/PerigeeSurface.hpp"

namespace tracking::reco {

  class TruthSeedProcessor : public framework::Producer {

 public :


    TruthSeedProcessor(const std::string &name, framework::Process &process);

    ~TruthSeedProcessor();

    void onProcessStart() final override;
    void onProcessEnd() final override;
    void configure(framework::config::Parameters& parameters) final override;
    void produce(framework::Event &event);

 private:

    //TODO::Address the geometry context properly
    Acts::GeometryContext gctx_;
    
    //Debug flag
    bool debug_{false};
    
    //Event counter
    int nevents_{0};

    //Output seed collection name
    std::string out_trk_coll_name_{"TruthSeeds"};
    
    //Processing time counter
    double processing_time_{0.};

    //pdgIDs of the particles we want to select for the seeds
    std::vector<int> pdgIDs_{11};

    //Which scoring plane hits to use for the truth seeds generation
    std::string scoring_hits_{"TargetScoringPlaneHits"};

    //Sim hits to check if the truth seed is findable
    std::string sim_hits_{"RecoilSimHits"};

    //Minimum number of hits left in a tracker to consider the seed as findable
    int n_min_hits_{7};
    
    //Min cut on the z of the scoring hit. It could be used to clean the scoring hits if wanted.
    float z_min_{-999};

    //Only select a particular trackID
    int track_id_{-999};

    //Ask a minimum pz for the seeds
    double pz_cut_{-9999};

    //Ask a minimum p for the seeds
    double p_cut_{0.};
    
    
  };
}


#endif

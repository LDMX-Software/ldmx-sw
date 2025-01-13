#pragma once

//--- Framework ---//
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"
#include "Framework/RandomNumberSeedService.h"

//--- C++ ---//
#include <memory>
#include <random>

//--- LDMX ---//
#include "Tracking/Reco/TrackingGeometryUser.h"

//--- Tracking ---//
#include "TFile.h"
#include "TTree.h"
#include "Tracking/Event/Measurement.h"
#include "Tracking/Event/ReducedTrack.h"

namespace tracking {
namespace reco {

class ReducedTrackFinder : public TrackingGeometryUser {
public:
    /**
     * Constructor.
     *
     * @param name The name of the instance of this object.
     * @param process The process running this producer.
     */
    ReducedTrackFinder(const std::string &name, framework::Process &process);
    
    /// Destructor
    ~ReducedTrackFinder();
    
    /**
     *
     */
    void onProcessStart() override;
    /**
     *
     */
    void onProcessEnd() override;
    
    /**
     * Configure the processor using the given user specified parameters.
     *
     * @param parameters Set of parameters used to configure this processor.
     */
    void configure(framework::config::Parameters &parameters) override;
    
    /**
     * Run the processor
     *
     * @param event The event to process.
     */
    void produce(framework::Event &event) override;
    
private:
    int nevents_{0};
    double processing_time_{0.};
    
    // The output track collection
    std::string out_trk_collection_{"ReducedTracks"};
    
    // The seed track collection
    std::string seed_coll_name_{"ReducedSeedTracks"};
    
    int nseeds_{0};
    int ntracks_{0};
    int eventnr_{0};
    
    std::vector<ldmx::ReducedTrack> findTracks(const std::vector<ldmx::ReducedTrack>& trackSeeds);
    bool isPositionUsed(const ldmx::Measurement& measurement, const std::set<std::tuple<float, float, float>>& usedSensorPositions);
    
};  // ReducedTrackFinder

}  // namespace reco
}  // namespace tracking

#pragma once

//---< Framework >---//
#include "Framework/Configure/Parameters.h"
#include "Framework/Event.h"
#include "Framework/EventProcessor.h"

//---< Tracking >---//
#include "Tracking/Sim/LdmxSpacePoint.h"
#include "Tracking/Sim/SeedToTrackParamMaker.h"
#include "Tracking/Sim/TrackingUtils.h"

//---< SimCore >---//
#include "SimCore/Event/SimTrackerHit.h"

//---< STD C++ >---//

#include <iostream>

//---< ACTS >---//
#include "Acts/Definitions/Algebra.hpp"
#include "Acts/MagneticField/MagneticFieldContext.hpp"
#include "Acts/Seeding/EstimateTrackParamsFromSeed.hpp"
#include "Acts/Seeding/Seed.hpp"
#include "Acts/Seeding/SeedFilter.hpp"
#include "Acts/Seeding/SpacePointGrid.hpp"
#include "Acts/Utilities/CalibrationContext.hpp"
#include "Acts/Utilities/Intersection.hpp"

//--- LDMX ---//
#include "TFile.h"
#include "TTree.h"
#include "Tracking/Event/Measurement.h"
#include "Tracking/Reco/TrackingGeometryUser.h"
#include "Tracking/Reco/TruthMatchingTool.h"
#include "Tracking/Event/StraightTrack.h"
#include "Ecal/Event/EcalHit.h"

namespace tracking {
namespace reco {

class LinearTruthTracking : public TrackingGeometryUser {
public:
    /**
     * Constructor.
     *
     * @param name The name of the instance of this object.
     * @param process The process running this producer.
     */
    LinearTruthTracking(const std::string& name, framework::Process& process);
    
    /// Destructor
    ~LinearTruthTracking();
    
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
    void configure(framework::config::Parameters& parameters) override;
    
    /**
     * Run the processor and create a collection of results which
     * indicate if a charge particle can be found by the recoil tracker.
     *
     * @param event The event to process.
     */
    void produce(framework::Event& event) override;
    
protected:
    ldmx::StraightTrack TruthTracker(const std::vector<ldmx::Measurement>& points, std::vector<std::array<double, 3>>& ecalPoints);
    
    std::vector<ldmx::Measurement> findTruthHits(const std::vector<ldmx::Measurement>& hitCollection);
   
    double calculateDistance(const std::array<double, 3> &point1, const std::array<double, 3> &point2);

    int uniqueSensorsHit(const std::vector<ldmx::Measurement> &digiPoints);
    
    std::tuple<double, double, double, double> fit3DLine(const std::vector<ldmx::Measurement>& points);
    double globalChiSquare(const std::vector<ldmx::Measurement>& points, double ax, double ay, double bx, double by);
    
    double processing_time_{0.};
    long nevents_{0};
    unsigned int ntruth_{0};
    long nmissing_{0};
    
    std::vector<double> recoil_truth_uncertainty_{0.006, 0.12};

    /// The name of the output collection of seeds to be stored.
    std::string out_trk_collection_{"LinearRecoilTruthTracks"};
    /// The name of the input hits collection to use in finding seeds..
    std::string input_hits_collection_{"DigiRecoilSimHits"};
    /// The name of the tagger Tracks (only for Recoil Seeding)
    std::string input_recHits_collection_{"EcalRecHits"};
       
    // Truth Matching tool
    std::shared_ptr<tracking::sim::TruthMatchingTool> truthMatchingTool_ = nullptr;
    
};  // LinearTruthTracking

}  // namespace reco
}  // namespace tracking

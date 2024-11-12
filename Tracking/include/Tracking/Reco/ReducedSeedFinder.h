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
#include "Tracking/Event/ReducedTrack.h"
#include "Ecal/Event/EcalHit.h"

namespace tracking {
namespace reco {

class ReducedSeedFinder : public TrackingGeometryUser {
public:
    /**
     * Constructor.
     *
     * @param name The name of the instance of this object.
     * @param process The process running this producer.
     */
    ReducedSeedFinder(const std::string& name, framework::Process& process);
    
    /// Destructor
    ~ReducedSeedFinder();
    
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
    ldmx::ReducedTrack SeedTracker(const std::array<double, 3> recoilOne, const std::array<double, 3> recoilTwo, const std::array<double, 3> ecalOne);
    
    std::pair<std::vector<std::array<double, 3>>, std::vector<std::array<double, 3>>> combineMultiGlobalHits(const std::vector<std::array<double, 4>> &hitCollection);
    std::vector<std::array<double, 3>> weightedAverage(const std::vector<std::array<double, 4>> &layer1, const std::vector<std::array<double, 4>> &layer2);
    std::tuple<double, double, double, double> fit3DLine(const std::array<double, 3> &firstRecoil, const std::array<double, 3> &secondRecoil, const std::array<double, 3> &ECal);
    
    double calculateDistance(const std::array<double, 3> &point1, const std::array<double, 3> &point2);
    double globalChiSquare(const std::array<double, 3> &firstSensor, const std::array<double, 3> &secondSensor, const std::array<double, 3> &ecalHit, double ax, double ay, double bx, double by);
    int uniqueSensorsHit(const std::vector<std::array<double, 4>> &digiPoints);
    
    double processing_time_{0.};
    long nevents_{0};
    unsigned int ntracks_{0};
    
    /// The name of the output collection of seeds to be stored.
    std::string out_seed_collection_{"ReducedSeedTracks"};
    /// The name of the input hits collection to use in finding seeds..
    std::string input_hits_collection_{"DigiRecoilSimHits"};
    /// The name of the tagger Tracks (only for Recoil Seeding)
    std::string input_recHits_collection_{"EcalRecHits"};
    /// Location of the perigee for the helix track parameters.
    std::vector<double> perigee_location_{0., 0., 0};
    
    double piover2_{1.5708};
    
    /// PhiRange
    double phicut_{0.1};
    
    /// ThetaRange
    double thetacut_{0.2};
    
    /// loc0 / loc1 cuts
    double loc0cut_{0.1};
    double loc1cut_{0.3};
    
    double ecal_uncertainty_{3.87};
    double ecal_distance_threshold_{10.0};
    
    std::vector<double> recoil_uncertainty_{0.006, 0.12};
    
    std::vector<double> zpos_digi_tot_;
    std::vector<double> xpos_digi_tot_;
    std::vector<double> ypos_digi_tot_;
    std::vector<double> edep_digi_;
    
    std::vector<double> ecal_end_x_;
    std::vector<double> ecal_end_y_;
    std::vector<double> ecal_end_z_;
    
    std::vector<std::array<double, 4>> digiPoints_;
    std::vector<std::array<double, 3>> firstLayerEcalRecHits_;
    
    // Check failures
    //  long ndoubles_{0};
    long nmissing_{0};
    //  long nfailphi_{0};
    //  long nfailtheta_{0};
    
};  // SeedFinderProcessor

}  // namespace reco
}  // namespace tracking

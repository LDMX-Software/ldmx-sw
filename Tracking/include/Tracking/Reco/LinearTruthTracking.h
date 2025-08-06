#pragma once

//---< Framework >---//
#include "Framework/Configure/Parameters.h"
#include "Framework/Event.h"
#include "Framework/EventProcessor.h"

//---< SimCore >---//
#include "SimCore/Event/SimTrackerHit.h"

//---< STD C++ >---//
#include <iostream>

//--- LDMX ---//
#include "Ecal/Event/EcalHit.h"
#include "TFile.h"
#include "TTree.h"
#include "Tracking/Event/Measurement.h"
#include "Tracking/Event/StraightTrack.h"
#include "Tracking/Reco/TrackingGeometryUser.h"
#include "Tracking/Reco/TruthMatchingTool.h"

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
  virtual ~LinearTruthTracking() = default;
  /**
   * Output event statistics
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
  // Function to find recoil e- based on 4 recoil points with trackID = 1
  ldmx::StraightTrack truthTracker(
      const std::vector<ldmx::SimTrackerHit>& points,
      std::vector<std::array<double, 3>>& ecal_points);

  // Helper function: calculate distance between 2 3D points
  double calculateDistance(const std::array<double, 3>& point1,
                           const std::array<double, 3>& point2);

  // Fitting function: fit a straight line in 3D using 4 points (2 degrees of
  // freedom)
  std::tuple<double, double, double, double> fit3DLine(
      const std::vector<ldmx::SimTrackerHit>& points);

  // Calculate chi2 of the fit
  double globalChiSquare(const std::vector<ldmx::SimTrackerHit>& points,
                         double a_x, double a_y, double b_x, double b_y);

  double processing_time_{0.};
  long n_events_{0};
  unsigned int n_truth_{0};
  long n_missing_{0};
  long n_empty_{0};

  // Assuming rLDMX v1 geometry
  double ecal_first_layer_z_threshold_{250.0};

  /// The name of the output collection of seeds to be stored.
  std::string out_trk_collection_{"LinearRecoilTruthTracks"};
  /// The name of the input hits collection to use in finding seeds..
  std::string input_hits_collection_{"RecoilSimHits"};
  /// The name of the tagger Tracks (only for Recoil Seeding)
  std::string input_rec_hits_collection_{"EcalRecHits"};
  std::string input_pass_name_{""};

};  // LinearTruthTracking

}  // namespace reco
}  // namespace tracking

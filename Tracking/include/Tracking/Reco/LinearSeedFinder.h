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

class LinearSeedFinder : public TrackingGeometryUser {
 public:
  /**
   * Constructor.
   *
   * @param name The name of the instance of this object.
   * @param process The process running this producer.
   */
  LinearSeedFinder(const std::string &name, framework::Process &process);

  /// Destructor
  virtual ~LinearSeedFinder() = default;
  /**
   * Setup the truth matching
   */
  void onProcessStart() override;
  /**
   * Output event statistics
   */
  void onProcessEnd() override;
  /**
   * Configure the processor using the given user specified parameters.
   *
   * @param parameters Set of parameters used to configure this processor.
   */
  void configure(framework::config::Parameters &parameters) override;

  /**
   * Run the processor and create a collection of results which
   * indicate if a charge particle can be found by the recoil tracker.
   *
   * @param event The event to process.
   */
  void produce(framework::Event &event) override;

 protected:
  // Function to find seeds based on 2 Recoil points and 1 EcalRecHit
  ldmx::StraightTrack SeedTracker(
      const std::tuple<std::array<double, 3>, ldmx::Measurement,
                       std::optional<ldmx::Measurement>>
          recoil_one,
      const std::tuple<std::array<double, 3>, ldmx::Measurement,
                       std::optional<ldmx::Measurement>>
          recoil_two,
      const std::array<double, 3> ecal_one);

  // Function to combine Recoil layer points into "real" sensor points
  std::pair<std::vector<std::tuple<std::array<double, 3>, ldmx::Measurement,
                                   std::optional<ldmx::Measurement>>>,
            std::vector<std::tuple<std::array<double, 3>, ldmx::Measurement,
                                   std::optional<ldmx::Measurement>>>>
  combineMultiGlobalHits(const std::vector<ldmx::Measurement> &hit_collection);

  // Function to do weighted averaging when combining Recoil layer points
  std::vector<std::tuple<std::array<double, 3>, ldmx::Measurement,
                         std::optional<ldmx::Measurement>>>
  midPointCalculation(const std::vector<ldmx::Measurement> &layer1,
                      const std::vector<ldmx::Measurement> &layer2);

  // Fitting function: fit a straight line in 3D using 3 points (1 degree of
  // freedom)
  std::tuple<double, double, double, double, std::vector<double>> fit3DLine(
      const std::array<double, 3> &first_recoil,
      const std::array<double, 3> &second_recoil,
      const std::array<double, 3> &ecal);

  // Helper function: calculate distance between 2 3D points
  double calculateDistance(const std::array<double, 3> &point1,
                           const std::array<double, 3> &point2);

  // Calculate chi2 of the fit
  double globalChiSquare(const std::array<double, 3> &first_sensor,
                         const std::array<double, 3> &second_sensor,
                         const std::array<double, 3> &ecal_hit, double a_x,
                         double a_y, double b_x, double b_y);

  // Function to find the number of unique layers hit (to determine if we have
  // enough points to fit)
  int uniqueLayersHit(const std::vector<ldmx::Measurement> &digi_points);

  double processing_time_{0.};
  long n_events_{0};
  unsigned int n_seeds_{0};

  /// The name of the output collection of seeds to be stored.
  std::string out_seed_collection_{"LinearRecoilSeedTracks"};
  /// The name of the input hits collection to use in finding seeds..
  std::string input_hits_collection_{"DigiRecoilSimHits"};
  /// The name of the tagger Tracks (only for Recoil Seeding)
  std::string input_rec_hits_collection_{"EcalRecHits"};
  std::string input_pass_name_{""};

  double ecal_uncertainty_{3.87};
  // Max distance from RecHit for valid track
  double ecal_distance_threshold_{10.0};

  // Assuming rLDMX_v1 geometry
  double layer12_midpoint_{12.5};
  double layer23_midpoint_{20.0};
  double layer34_midpoint_{27.5};
  double ecal_first_layer_z_threshold_{250.0};

  std::vector<double> recoil_uncertainty_{0.006, 5.7735};

  // Check failures
  long n_missing_{0};

  // Truth Matching tool
  std::shared_ptr<tracking::sim::TruthMatchingTool> truth_matching_tool_ =
      nullptr;

};  // SeedFinderProcessor

}  // namespace reco
}  // namespace tracking

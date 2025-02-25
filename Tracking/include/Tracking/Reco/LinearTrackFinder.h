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
#include "Tracking/Event/StraightTrack.h"

namespace tracking {
namespace reco {

class LinearTrackFinder : public TrackingGeometryUser {
 public:
  /**
   * Constructor.
   *
   * @param name The name of the instance of this object.
   * @param process The process running this producer.
   */
  LinearTrackFinder(const std::string &name, framework::Process &process);
   
  /// Destructor
  virtual ~LinearTrackFinder() = default;
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
  int n_events_{0};
  double processing_time_{0.};

  // The output track collection
  std::string out_trk_collection_{"LinearRecoilTracks"};

  // The seed track collection
  std::string seed_collection_{"LinearRecoilSeedTracks"};

  int n_seeds_{0};
  int n_tracks_{0};

  // Find tracks from seeds by taking the seeds with lowest chi2 for each RecHit
  std::vector<ldmx::StraightTrack> findTracks(
      const std::vector<ldmx::StraightTrack> &track_seeds);

  // Helper function to check if a measurement's position is already used
  bool isPositionUsed(
      const ldmx::Measurement &measurement,
      const std::set<std::tuple<float, float, float>> &used_sensor_positions);

};  // LinearTrackFinder

}  // namespace reco
}  // namespace tracking

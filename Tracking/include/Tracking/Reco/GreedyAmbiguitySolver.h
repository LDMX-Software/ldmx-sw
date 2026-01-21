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

//--- ACTS ---//

// Utils and Definitions
#include "Acts/Definitions/Common.hpp"
#include "Acts/Utilities/Logger.hpp"

// geometry
#include "Acts/Geometry/GeometryContext.hpp"

// geometry
#include <Acts/Geometry/TrackingGeometry.hpp>

#include "Acts/Geometry/GeometryIdentifier.hpp"

//--- Tracking ---//
#include "Tracking/Event/Measurement.h"
#include "Tracking/Event/Track.h"
#include "Tracking/Reco/TrackExtrapolatorTool.h"
#include "Tracking/Sim/IndexSourceLink.h"
#include "Tracking/Sim/MeasurementCalibrator.h"
#include "Tracking/Sim/TrackingUtils.h"

//--- Interpolated magnetic field ---//
#include "Tracking/Sim/BFieldXYZUtils.h"

namespace tracking {
namespace reco {

/**
 * Minimal example of a processor.
 *
 * This processor will loop over all of the ECal hits in an event and
 * print out their details.
 */
class GreedyAmbiguitySolver final : public TrackingGeometryUser {
 public:
  /**
   * Constructor.
   *
   * @param name Name for this instance of the class.
   * @param process The Process class associated with EventProcessor,
   * provided by the framework.
   */
  GreedyAmbiguitySolver(const std::string& name, framework::Process& process);

  /// Destructor
  virtual ~GreedyAmbiguitySolver() = default;

  /**
   * onNewRun is the first function called for each processor
   * *after* the conditions are fully configured and accessible.
   * This is where you could create single-processors, multi-event
   * calculation objects.
   */
  void onNewRun(const ldmx::RunHeader& rh) override;

  /**
   * Configure the processor using the given user specified parameters.
   *
   * The user specified parameters that are availabed are defined
   * in the python configuration class. Look at the my_processor.py
   * module of the EventProc python for the python structure.
   *
   * @param parameters Set of parameters used to configure this processor.
   */
  void configure(framework::config::Parameters& parameters) override;

  /**
   * Process the event and put new data products into it.
   *
   * @param event The event to process.
   */
  void produce(framework::Event& event) override;

 private:
  /// Maximum amount of shared hits per track.
  std::uint32_t maximum_shared_hits_;
  /// Maximum number of iterations
  std::uint32_t maximum_iterations_;

  /// Minimum number of measurement to form a track.
  std::size_t n_meas_min_;

  /// Name of the output cleaned Track collection.
  std::string out_trk_collection_;

  /// Track collection to be cleaned.
  std::string track_collection_;

  /// Measurements collection in the tracker
  std::string meas_collection_;

  /// Minimum momentum of the cleaned tracks (MeV)
  double min_p_;

  /// Input pass name
  std::string input_pass_name_;

  struct State {
    std::size_t number_of_tracks_{};

    std::vector<int> track_tips_;
    std::vector<float> track_chi2_;
    std::vector<std::vector<std::size_t>> measurements_per_track_;

    // TODO consider boost 1.81 unordered_flat_map
    boost::container::flat_map<std::size_t,
                               boost::container::flat_set<std::size_t>>
        tracks_per_measurement_;
    std::vector<std::size_t> shared_measurements_per_track_;

    // TODO consider boost 1.81 unordered_flat_map
    boost::container::flat_set<std::size_t> selected_tracks_;
  };

  /// @param tracks The input track container.
  /// @param state An empty state object which is expected to be default
  /// constructed.
  /// @param sourceLinkHash A functor to acquire a hash from a given source
  /// link.
  /// @param sourceLinkEquality A functor to check equality of two source links.
  template <typename geometry_t, typename source_link_hash_t,
            typename source_link_equality_t>
  void computeInitialState(std::vector<ldmx::Track> tracks,
                           std::vector<ldmx::Measurement> measurements,
                           State& state, geometry_t& tg,
                           source_link_hash_t&& sourceLinkHash,
                           source_link_equality_t&& sourceLinkEquality) const;

  /// Updates the state iteratively by evicting one track after the other until
  /// the final state conditions are met.
  ///
  /// @param state A state object that was previously filled by the
  /// initialization.
  void resolve(State& state);

  /// @param state A state object that was previously filled by the
  /// initialization.
  /// @param iTrack
  void removeTrack(State& state, std::size_t iTrack) const;

  /*
  /// @param a
  std::size_t sourceLinkHash(const Acts::SourceLink& a);

  /// @param a
  /// @param b
  bool sourceLinkEquality(const Acts::SourceLink& a, const Acts::SourceLink& b);
  */

};  // MyProcessor

}  // namespace reco
}  // namespace tracking
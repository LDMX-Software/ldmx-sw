#pragma once

//--- Framework ---//
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"
#include "Framework/Logger.h"
#include "SimCore/Event/SimParticle.h"
#include "SimCore/Event/SimTrackerHit.h"

// --- Tracking --- //
#include "Tracking/Event/Track.h"
#include "Tracking/Event/TruthTrack.h"
#include "Tracking/Reco/TrackExtrapolatorTool.h"
#include "Tracking/Reco/TrackingGeometryUser.h"
#include "Tracking/Sim/TrackingUtils.h"

// --- ACTS --- //
#include <random>

#include "Acts/Definitions/Algebra.hpp"
#include "Acts/Definitions/TrackParametrization.hpp"
#include "Acts/EventData/TrackParameters.hpp"
#include "Acts/Propagator/EigenStepper.hpp"
#include "Acts/Propagator/Navigator.hpp"
#include "Acts/Propagator/Propagator.hpp"
#include "Acts/Surfaces/PerigeeSurface.hpp"
#include "Tracking/Sim/BFieldXYZUtils.h"

using TruthPropagator = Acts::Propagator<Acts::EigenStepper<>, Acts::Navigator>;

namespace tracking::reco {

/**
 * Creates a track from a seed from the tagger or recoil tracker. 
 * The track paramterization is expressed in the ACTS coordinate system, 
 * and the track is extrapolated to a surface depending on if its from the tagger or recoil.
 * Optional smearing is applied for the seed collections.
 */
class TruthTrackProcessor : public TrackingGeometryUser {
 public:
  /**
   * Constructor.
   *
   * @param name Name for this instance of the class.
   * @param process The Process class associated with EventProcessor, provided
   * by the framework.
   */
  TruthTrackProcessor(const std::string& name, framework::Process& process);

  /// Destructor
  virtual ~TruthTrackProcessor() = default;

  /**
   * Callback for the EventProcessor to configure itself from the
   * given set of parameters.
   *
   * The parameters a processor has access to are the member variables
   * of the python class in the sequence that has class_name equal to
   * the EventProcessor class name.
   *
   * @param parameters Parameters for configuration.
   */
  void configure(framework::config::Parameters& parameters) override;

  /**
   * onNewRun is the first function called for each processor
   * *after* the conditions are fully configured and accessible.
   * This is where you could create single-processors, multi-event
   * calculation objects.
   */
  void onNewRun(const ldmx::RunHeader& rh) override;

  /**
   * Main loop that creates the seed tracks for both the tagger and recoil
   * tracker.
   *
   * @param event The event containing the collections to process.
   */
  void produce(framework::Event& event) override;

 private:

  /*
   * Create a truth track using the track state of a track.
   *
   * @param ts The track state at the target surface for the track.
   */
  void createTruthTrack(const ldmx::Track::TrackState& ts, ldmx::Track& trk,
                        const std::shared_ptr<Acts::Surface>& target_surface);

  
  /** Create a track seed from a truth track applying a smearing to the truth
   * parameters as well as an inflation to the covariance matrix.
   * @param tt TruthTrack to be used to form a seed
   * @return seed The seed track
   */

  ldmx::Track seedFromTruth(const ldmx::Track& tt, bool seed_smearing);

  ldmx::Track recoilFullSeed(
      const ldmx::SimParticle& particle, const int trackID,
      const ldmx::SimTrackerHit& hit, const ldmx::SimTrackerHit& ecal_hit,
      const std::map<int, std::vector<int>>& hit_count_map,
      const std::shared_ptr<Acts::Surface>& origin_surface,
      const std::shared_ptr<Acts::Surface>& target_surface,
      const std::shared_ptr<Acts::Surface>& ecal_surface);

  /**
   * This method retrieves the beam electron and forms a full seed
   * The seed parameters are the truth parameters from the beam electron stored
   * at the beam origin .
   * Linear extrapolations are done from the origin of the particle to the
   * reference surfaces This track also contains the list of hits belonging to
   * the beam electron on the sensitive surfaces on the tagger tracker, for
   * acceptance studies
   * @param beam_electron  : the beam electron track
   * @param trackID        : the unique ID of the track
   * electron particle survived
   * @param origin_surface : where to express the track origin parameters. Can
   * be perigee, plane...
   * @param target_surface : the target surface for the truth target state
   */
  ldmx::Track taggerFullSeed(
      const ldmx::Track& beam_electron, const int trackID,
      const std::shared_ptr<Acts::Surface>& origin_surface,
      const std::shared_ptr<Acts::Surface>& target_surface);

  /// The ACTS geometry context properly
  Acts::GeometryContext gctx_;


  /// Pass name for the sim hit collections
  std::string input_pass_name_{""};

  std::string input_tagger_truth_collection_;
  std::string input_recoil_truth_collection_;
  std::string input_beam_electrons_collection_;

  // skip the tagger tracker
  bool skip_tagger_{false};

  // skip the recoil tracker
  bool skip_recoil_{false};

  std::unique_ptr<const TruthPropagator> propagator_;

  // Track Extrapolator Tool
  std::shared_ptr<tracking::reco::TrackExtrapolatorTool<TruthPropagator>>
      trk_extrap_;

  /// Path to the magnetic field map
  std::string field_map_{""};

  //--- Smearing ---//

  std::default_random_engine generator_;
  std::shared_ptr<std::normal_distribution<float>> normal_;

  bool seed_smearing_{false};

  std::vector<double> d0smear_;
  std::vector<double> z0smear_;
  double phismear_;
  double thetasmear_;
  double relpsmear_;
  std::vector<double> rel_smearfactors_;
  std::vector<double> inflate_factors_;
  std::vector<double> beam_origin_{-880.1, -44., 0.};
  int particle_hypothesis_;

  std::string beam_electrons_collection_;
  std::string tagger_truth_collection_;
  std::string recoil_truth_collection_;
  std::string tagger_seeds_collection_;
  std::string recoil_seeds_collection_;
};
}  // namespace tracking::reco
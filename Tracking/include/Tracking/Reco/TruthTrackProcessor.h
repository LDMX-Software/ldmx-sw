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
 * Produce final truth tracks from truth seeds by propagating to the target
 * surface, applying more hit and momentum cuts, and applying ecal states to  
 * recoil tracks
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
   * Callback for the EventProcessor to take any necessary action when the
   * processing of events starts. For this class, the callback is used to
   * retrieve the GeometryContext from ACTS.
   */
  void onProcessStart() override {};

  /**
   * onNewRun is the first function called for each processor
   * *after* the conditions are fully configured and accessible.
   * This is where you could create single-processors, multi-event
   * calculation objects.
   */
  void onNewRun(const ldmx::RunHeader& rh) override;

  /**
   * Main loop that produces final truth tracks for both the tagger and recoil
   * tracker.
   *
   * @param event The event containing the collections to process.
   */
  void produce(framework::Event& event) override;

 private:

  /// The ACTS geometry context properly
  Acts::GeometryContext gctx_;

  /// Pass name for the input seed collections
  std::string input_pass_name_{""};

  std::string sim_particles_coll_name_;
  std::string sim_particles_passname_;

  std::string ecal_sp_coll_name_{"EcalScoringPlaneHits"};
  std::string sp_pass_name_{""};

  /**
   * Minimum number of hits left in the tagger tracker to consider the track
   * as findable
   */
  int n_min_hits_tagger_{7};

  /**
   * Minimum number of hits left in the recoil tracker to consider the seed
   * as findable
   */
  int n_min_hits_recoil_{7};

  /// Ask for a minimum pz for the seeds
  double pz_cut_{-9999};

  /// Ask for a minimum p for the seeds
  double p_cut_{0.};

  /// Ask for a maximum p for the seeds
  double p_cut_max_{100000.};

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
  int particle_hypothesis_;

  std::string tagger_seeds_collection_;
  std::string recoil_seeds_collection_;
  std::string tagger_tracks_collection_;
  std::string recoil_tracks_collection_;
};
}  // namespace tracking::reco
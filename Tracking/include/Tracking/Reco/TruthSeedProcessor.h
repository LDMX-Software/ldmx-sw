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
#include "Tracking/Reco/TrackingGeometryUser.h"
#include "Tracking/Reco/TrackExtrapolatorTool.h"
#include "Tracking/Sim/TrackingUtils.h"

// --- ACTS --- //
#include "Acts/Definitions/Algebra.hpp"
#include "Acts/Definitions/TrackParametrization.hpp"
#include "Acts/EventData/TrackParameters.hpp"
#include "Acts/Surfaces/PerigeeSurface.hpp"
#include "Acts/Propagator/EigenStepper.hpp"
#include "Acts/Propagator/Navigator.hpp"
#include "Acts/Propagator/Propagator.hpp"
#include "Tracking/Sim/BFieldXYZUtils.h"

#include <random>

using TruthPropagator = Acts::Propagator<Acts::EigenStepper<>, Acts::Navigator>;

namespace tracking::reco {

/**
 * Create a track seed using truth information extracted from the corresponding
 * SimParticle or SimTrackerHit. When creating seeds in the Tagger tracker,
 * the SimParticle associated with the incident electron (trackID == 1) is used
 * to create the seed from the parameters (x_, y_, z_, px, py, pz, q) at the
 * vertex. For the Recoil tracker, since the electron is produced
 * upstream, the SimParticle can't be used to get any parameters at the target.
 * In this case, the target scoring plane hits are used to extract the
 * parameters above.
 */
class TruthSeedProcessor : public TrackingGeometryUser {
 public:
  /**
   * Constructor.
   *
   * @param name Name for this instance of the class.
   * @param process The Process class associated with EventProcessor, provided
   * by the framework.
   */
  TruthSeedProcessor(const std::string& name, framework::Process& process);

  /// Destructor
  virtual ~TruthSeedProcessor() = default;

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
   * Main loop that creates the seed tracks for both the tagger and recoil
   * tracker.
   *
   * @param event The event containing the collections to process.
   */
  void produce(framework::Event& event) override;

 private:
  /**
   * Create a mapping from the selected scoring plane hit objects to the number
   * of hits they associated particle creates in the tracker.
   * @param sim_hits vector
   * @param hit_count_map filled with the hits lefts by each track
   */
  void makeHitCountMap(const std::vector<ldmx::SimTrackerHit>& sim_hits,
                       std::map<int, std::vector<int>>& hit_count_map);

  /**
   * Use the vertex position of the SimParticle to extract
   * (x_, y_, z_, px, py, pz, q) and create a track seed.
   *
   * @param particle The SimParticle to make a seed from.
   */
  void createTruthTrack(const ldmx::SimParticle& particle, ldmx::Track& trk);

  /**
   * Use the scoring plane hit at the target to extract
   * (x_, y_, z_, px, py, pz) and create a track seed. In this case, the
   * SimParticle is used to extract the charge of the particle.
   *
   * @param particle The SimParticle to extract the charge from.
   * @param hit The SimTrackerHit used to create the seed.
   */
  void createTruthTrack(const ldmx::SimParticle& particle,
                        const ldmx::SimTrackerHit& hit, ldmx::Track& trk);

  /**
   * Create a seed track from the given position, momentum and charge.
   *
   * @param pos_ The position at which the particle was created.
   * @param p The momentum of the particle at the point of creation.
   * @param charge The charge of the particle.
   */
  void createTruthTrack(const std::vector<double>& pos_vec,
                        const std::vector<double>& p_vec, int charge,
                        ldmx::Track& trk);

  /**
   * Use the vertex position of the SimParticle to extract
   * (x_, y_, z_, px, py, pz, q) and create a track seed.
   *
   * @param particle The SimParticle to make a seed from.
   * @param target_surface The ACTS surface for the track parameters.
   */
  void createTruthTrack(const ldmx::SimParticle& particle, ldmx::Track& trk,
                        const std::shared_ptr<Acts::Surface>& target_surface);

  /**
   * Use the scoring plane hit at the target to extract
   * (x_, y_, z_, px, py, pz) and create a track seed. In this case, the
   * SimParticle is used to extract the charge of the particle.
   *
   * @param particle The SimParticle to extract the charge from.
   * @param hit The SimTrackerHit used to create the seed.
   * @param target_surface The ACTS surface for the track parameters.
   */
  void createTruthTrack(const ldmx::SimParticle& particle,
                        const ldmx::SimTrackerHit& hit, ldmx::Track& trk,
                        const std::shared_ptr<Acts::Surface>& target_surface);

  /**
   * Create a seed track from the given position, momentum and charge.
   *
   * @param pos_ The position at which the particle was created.
   * @param p The momentum of the particle at the point of creation.
   * @param charge The charge of the particle.
   * @param target_surface The ACTS surface for the track parameters.
   */
  void createTruthTrack(const std::vector<double>& pos_vec,
                        const std::vector<double>& p_vec, int charge,
                        ldmx::Track& trk,
                        const std::shared_ptr<Acts::Surface>& target_surface);

    /**
     * Create a full truth seed for the recoil tracker.
     *
     * @param particle The SimParticle for the track.
     * @param trackID The track ID.
     * @param hit The tracking hit to extract parameters from.
     * @param hit_count_map The map of hit counts per track.
     * @param origin_surface The origin surface (target).
     */
  ldmx::Track recoilFullSeed(
      const ldmx::SimParticle& particle, const int trackID,
      const ldmx::SimTrackerHit& hit,
      const std::map<int, std::vector<int>>& hit_count_map,
      const std::shared_ptr<Acts::Surface>& origin_surface);

  /**
   * Create a full truth seed for the tagger tracker.
   *
   * @param beam_electron The beam electron SimParticle.
   * @param trackID The track ID.
   * @param hit The tracking hit to extract parameters from.
   * @param hit_count_map The map of hit counts per track.
   * @param origin_surface The origin surface.
   */
  ldmx::Track taggerFullSeed(
      const ldmx::SimParticle& beam_electron, const int trackID,
      const ldmx::SimTrackerHit& hit,
      const std::map<int, std::vector<int>>& hit_count_map,
      const std::shared_ptr<Acts::Surface>& origin_surface);

  /**
   * Create a seed track from a truth track applying a smearing to the truth
   * parameters as well as an inflation to the covariance matrix.
   *
   * @param tt TruthTrack to be used to form a seed
   * @param seed_smearing Whether to apply smearing
   * @return seed The seed track
   */
  ldmx::Track seedFromTruth(const ldmx::Track& tt, bool seed_smearing);

  /**
   * Filter that checks if a scoring plane passes specified momentum cuts as
   * well as if the associated SimParticle hits the ECal.
   *
   * @param hit The target scoring plane hit to check.
   * @param ecal_sp_hits The ECal scoring plane hit used to check if the
   * associated particle hits the ECal.
   */
  bool scoringPlaneHitFilter(
      const ldmx::SimTrackerHit& hit,
      const std::vector<ldmx::SimTrackerHit>& ecal_sp_hits);

  /// The ACTS geometry context properly
  Acts::GeometryContext gctx_;

  /// pdg_ids of the particles we want to select for the seeds
  std::vector<int> pdg_ids_{11};

  /// Which scoring plane hits to use for the truth seeds generation
  std::string scoring_hits_coll_name_{"TargetScoringPlaneHits"};
  std::string sp_pass_name_{""};

  /// Sim hits to check if the truth seed is findable
  std::string tagger_sim_hits_coll_name_{"TaggerSimHits"};

  /// Sim hits to check if the truth seed is findable
  std::string recoil_sim_hits_coll_name_{"RecoilSimHits"};

  /// Pass name for the sim hit collections
  std::string input_pass_name_{""};

  std::string sim_particles_coll_name_;
  std::string sim_particles_passname_;

  /**
   * Minimum number of hits left in the recoil tracker to consider the seed
   * as findable
   */
  int n_min_hits_tagger_{7};

  /**
   * Minimum number of hits left in the recoil tracker to consider the seed
   * as findable
   */
  int n_min_hits_recoil_{7};

  /**
   * Min cut on the z_ of the scoring hit. It could be used to clean the scoring
   * hits if desired.
   */
  float z_min_{-999};

  /// Only select a particular trackID
  int track_id_{-999};

  /// Ask for a minimum p for the seeds
  double p_cut_{0.};

  // Ask for a minimum p for the seeds at the ecal (from truth)
  double p_cut_ecal_{-1.};

  // Use scoring plane for target truth tracks
  bool target_sp_{true};

  // Use scoring plane for recoil truth tracks
  bool recoil_sp_{true};

  // skip the tagger tracker
  bool skip_tagger_{false};

  // skip the recoil tracker
  bool skip_recoil_{false};

  // Maximum track id for hit to be selected from target scoring plane
  int max_track_id_{5};

  // In tracking frame: where do these numbers come from?
  // These numbers come from approximating the path of the beam up
  // until it is about to enter the first detector volume (TriggerPad1).
  // In detector coordinates, (x_,y_,z_) = (-21.7, -883) is
  // where the beam arrives (if no smearing is applied) and we simply
  // reorder these values so that they are in tracking coordinates.
  std::vector<double> beam_origin_{-883.0, -21.745876, 0.0};

  std::string beam_electrons_collection_;
  std::string tagger_seeds_collection_;
  std::string recoil_seeds_collection_;

  /// Path to the magnetic field map
  std::string field_map_{""};

  //--- Smearing and extrapolation ---//

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

  std::unique_ptr<const TruthPropagator> propagator_;

  // Track Extrapolator Tool
  std::shared_ptr<tracking::reco::TrackExtrapolatorTool<TruthPropagator>>
      trk_extrap_;
};
}  // namespace tracking::reco
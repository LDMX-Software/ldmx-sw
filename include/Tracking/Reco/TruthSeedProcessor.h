#pragma once

//--- Framework ---//
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"
#include "SimCore/Event/SimParticle.h"
#include "SimCore/Event/SimTrackerHit.h"

// --- Tracking --- //
#include "Tracking/Event/Track.h"
#include "Tracking/Sim/TrackingUtils.h"

// --- ACTS --- //
#include "Acts/Definitions/Algebra.hpp"
#include "Acts/Definitions/TrackParametrization.hpp"
#include "Acts/EventData/TrackParameters.hpp"
#include "Acts/EventData/detail/TransformationFreeToBound.hpp"
#include "Acts/Surfaces/PerigeeSurface.hpp"

namespace tracking::reco {

/**
 * Create a track seed using truth information extracted from the corresponding
 * SimParticle or SimTrackerHit. When creating seeds in the Tagger tracker,
 * the SimParticle associated with the incident electron (trackID == 1) is used
 * to create the seed from the parameters (x, y, z, px, py, pz, q) at the
 * vertex. For the Recoil tracker, since the electron is produced
 * upstream, the SimParticle can't be used to get any parameters at the target.
 * In this case, the target scoring plane hits are used to extract the
 * parameters above.
 */
class TruthSeedProcessor : public framework::Producer {
 public:
  /**
   * Constructor.
   *
   * @param name Name for this instance of the class.
   * @param process The Process class associated with EventProcessor, provided
   * by the framework.
   */
  TruthSeedProcessor(const std::string &name, framework::Process &process);

  /// Destructor
  ~TruthSeedProcessor() = default;

  /**
   * Callback for the EventProcessor to configure itself from the
   * given set of parameters.
   *
   * The parameters a processor has access to are the member variables
   * of the python class in the sequence that has className equal to
   * the EventProcessor class name.
   *
   * @param parameters Parameters for configuration.
   */
  void configure(framework::config::Parameters &parameters) final override;

  /**
   * Callback for the EventProcessor to take any necessary action when the
   * processing of events starts. For this class, the callback is used to
   * retrieve the GeometryContext from ACTS.
   */
  void onProcessStart() final override;

  /**
   * Main loop that creates the seed tracks for both the tagger and recoil
   * tracker.
   *
   * @param event The event containing the collections to process.
   */
  void produce(framework::Event &event) final override;

 private:
  /**
   * Use the vertex position of the SimParticle to extract
   * (x, y, z, px, py, pz, q) and create a track seed.
   *
   * @param particle The SimParticle to make a seed from.
   */
  ldmx::Track createSeed(const ldmx::SimParticle &particle);

  /**
   * Use the scoring plane hit at the target to extract
   * (x, y, z, px, py, pz) and create a track seed. In this case, the
   * SimParticle is used to extract the charge of the particle.
   *
   * @param particle The SimParticle to extract the charge from.
   * @param hit The SimTrackerHit used to create the seed.
   */
  ldmx::Track createSeed(const ldmx::SimParticle &particle,
                         const ldmx::SimTrackerHit &hit);

  /**
   * Create a seed track from the given position, momentum and charge.
   *
   * @param pos The position at which the particle was created.
   * @param p The momentum of the particle at the point of creation.
   * @param charge The charge of the particle.
   */
  ldmx::Track createSeed(const std::vector<double> &pos_vec,
                         const std::vector<double> &p_vec, int charge);

  /**
   * Filter that checks if a scoring plane passes specified momentum cuts as
   * well as if the associated SimParticle hits the ECal.
   *
   * @param hit The target scoring plane hit to check.
   * @param ecal_sp_hits The ECal scoring plane hit used to check if the
   * associated particle hits the ECal.
   */
  bool scoringPlaneHitFilter(
      const ldmx::SimTrackerHit &hit,
      const std::vector<ldmx::SimTrackerHit> &ecal_sp_hits);

  /// The ACTS geometry context properly
  Acts::GeometryContext gctx_;

  /// pdg_ids of the particles we want to select for the seeds
  std::vector<int> pdg_ids_{11};

  /// Which scoring plane hits to use for the truth seeds generation
  std::string scoring_hits_coll_name_{"TargetScoringPlaneHits"};

  /// Sim hits to check if the truth seed is findable
  std::string recoil_sim_hits_coll_name_{"RecoilSimHits"};

  /**
   * Minimum number of hits left in the recoil tracker to consider the seed
   * as findable
   */
  int n_min_hits_{7};

  /**
   * Min cut on the z of the scoring hit. It could be used to clean the scoring
   * hits if desired.
   */
  float z_min_{-999};

  /// Only select a particular trackID
  int track_id_{-999};

  /// Ask for a minimum pz for the seeds
  double pz_cut_{-9999};

  /// Ask for a minimum p for the seeds
  double p_cut_{0.};

  /// Ask for a maximum p for the seeds
  double p_cut_max_{100000.};

  // Ask for a minimum p for the seeds at the ecal (from truth)
  double p_cut_ecal_{-1.};
};
}  // namespace tracking::reco

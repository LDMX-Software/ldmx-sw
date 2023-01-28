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
class TruthSeedProcessor : public framework::Producer {
 public:
  TruthSeedProcessor(const std::string &name, framework::Process &process);

  /// Destructor
  ~TruthSeedProcessor() = default;

  void onProcessStart() final override;
  void onProcessEnd() final override;
  void configure(framework::config::Parameters &parameters) final override;
  void produce(framework::Event &event) final override;

 private:
  /**
   */
  ldmx::Track createSeed(const ldmx::SimParticle &particle);

  /**
   */
  ldmx::Track createSeed(const ldmx::SimTrackerHit &hit);

  /**
   */
  ldmx::Track createSeed(const std::vector<double> &pos,
                         const std::vector<double> &p, int pdg_id);

  /**
   */
  bool scoringPlaneHitFilter(
      const ldmx::SimTrackerHit &hit,
      const std::vector<ldmx::SimTrackerHit> &ecal_sp_hits);

  // TODO::Address the geometry context properly
  Acts::GeometryContext gctx_;

  // Event counter
  // int nevents_{0};

  // Processing time counter
  // double processing_time_{0.};

  // pdg_ids of the particles we want to select for the seeds
  std::vector<int> pdg_ids_{11};

  // Which scoring plane hits to use for the truth seeds generation
  std::string scoring_hits_{"TargetScoringPlaneHits"};

  // Sim hits to check if the truth seed is findable
  std::string sim_hits_{"RecoilSimHits"};

  // Minimum number of hits left in a tracker to consider the seed as findable
  int n_min_hits_{7};

  // Min cut on the z of the scoring hit. It could be used to clean the scoring
  // hits if wanted.
  float z_min_{-999};

  // Only select a particular trackID
  int track_id_{-999};

  // Ask a minimum pz for the seeds
  double pz_cut_{-9999};

  // Ask a minimum p for the seeds
  double p_cut_{0.};

  // Ask a maximum p for the seeds
  double p_cutMax_{100000.};

  // Ask a minimum p for the seeds at the ecal (from truth)
  double p_cutEcal_{-1.};

  // Apply a dedicated k0 selection
  double k0_sel_{false};
};
}  // namespace tracking::reco

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
//#include "Acts/Seeding/BinFinder.hpp"  .......   mg...I think these are
//unused?  they are gone in v36 #include "Acts/Seeding/BinnedSPGroup.hpp"
//.......   mg...I think these are unused?  they are gone in v36
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

namespace tracking {
namespace reco {

class SeedFinderProcessor : public TrackingGeometryUser {
 public:
  /**
   * Constructor.
   *
   * @param name The name of the instance of this object.
   * @param process The process running this producer.
   */
  SeedFinderProcessor(const std::string& name, framework::Process& process);

  /// Destructor
  ~SeedFinderProcessor();

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

  bool GroupStrips(const std::vector<ldmx::Measurement>& measurements,
                   const std::vector<int> strategy);

  void FindSeedsFromMap(ldmx::Tracks& seeds, const ldmx::Measurements& pmeas);

 private:
  ldmx::Track SeedTracker(const ldmx::Measurements& vmeas, double xOrigin,
                          const Acts::Vector3& perigee_location,
                          const ldmx::Measurements& pmeas_tgt);

  void LineParabolaToHelix(const Acts::ActsVector<5> parameters,
                           Acts::ActsVector<5>& helix_parameters,
                           Acts::Vector3 ref);

  Acts::Vector3 bField_;

  /* This is a temporary (working) solution to estimate the track parameters out
   * of the seeds Eventually we should move to what is in ACTS (I'm not happy
   * with what they did regarding this part atm)
   */

  std::shared_ptr<tracking::sim::SeedToTrackParamMaker> seed_to_track_maker_;

  double processing_time_{0.};
  long nevents_{0};
  unsigned int ntracks_{0};

  std::vector<double> inflate_factors_{1., 1., 1., 1., 1.};

  /// The name of the output collection of seeds to be stored.
  std::string out_seed_collection_{"SeedTracks"};
  /// The name of the input hits collection to use in finding seeds..
  std::string input_hits_collection_{"TaggerSimHits"};
  /// The name of the tagger Tracks (only for Recoil Seeding)
  std::string tagger_trks_collection_{"TaggerTracks"};
  /// Location of the perigee for the helix track parameters.
  std::vector<double> perigee_location_{-700., 0., 0};
  /// Minimum cut on the momentum of the seeds.
  double pmin_{0.05};

  /// Maximum cut on the momentum of the seeds.
  double pmax_{8};

  /// Max d0 allowed for the seeds.
  double d0max_{20.};

  /// Min d0 allowed for the seeds.
  double d0min_{20.};

  /// Max z0 allowed for the seeds.
  double z0max_{60.};

  double piover2_{1.5708};

  /// PhiRange
  double phicut_{0.1};

  /// ThetaRange
  double thetacut_{0.2};

  /// loc0 / loc1 cuts
  double loc0cut_{0.1};
  double loc1cut_{0.3};

  /// List of stragies for seed finding.
  std::vector<std::string> strategies_{};
  double bfield_{1.5};

  TFile* outputFile_;
  TTree* outputTree_;

  std::vector<float> xhit_;
  std::vector<float> yhit_;
  std::vector<float> zhit_;

  std::vector<float> b0_;
  std::vector<float> b1_;
  std::vector<float> b2_;
  std::vector<float> b3_;
  std::vector<float> b4_;

  // Check failures
  long ndoubles_{0};
  long nmissing_{0};
  long nfailpmin_{0};
  long nfailpmax_{0};
  long nfaild0min_{0};
  long nfaild0max_{0};
  long nfailz0max_{0};
  long nfailphi_{0};
  long nfailtheta_{0};

  // The measurements groups

  std::map<int, std::vector<const ldmx::Measurement*>> groups_map;
  std::array<const ldmx::Measurement*, 5> groups_array;

  // Truth Matching tool
  std::shared_ptr<tracking::sim::TruthMatchingTool> truthMatchingTool_ =
      nullptr;

};  // SeedFinderProcessor

}  // namespace reco
}  // namespace tracking

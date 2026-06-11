#pragma once

//--- Framework ---//
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"
#include "Framework/Logger.h"
#include "Framework/RandomNumberSeedService.h"

//--- C++ ---//
#include <memory>
#include <random>

//--- LDMX ---//
#include "Tracking/Reco/TrackingGeometryUser.h"

//--- ACTS ---//

// Utils and Definitions
#include "Acts/Definitions/Common.hpp"
#include "Acts/Definitions/TrackParametrization.hpp"
#include "Acts/Definitions/Units.hpp"
#include "Acts/EventData/TrackParameters.hpp"
#include "Acts/Utilities/Logger.hpp"

// geometry
#include "Acts/Geometry/GeometryContext.hpp"

// magfield
#include "Acts/MagneticField/MagneticFieldContext.hpp"
#include "Acts/MagneticField/MagneticFieldProvider.hpp"

// geometry
#include <Acts/Geometry/TrackingGeometry.hpp>

// propagation testing
#include "Tracking/EigenStepper.h"
#include "Acts/MagneticField/ConstantBField.hpp"
#include "Acts/Propagator/AbortList.hpp"
#include "Acts/Propagator/ActionList.hpp"
#include "Acts/Propagator/DenseEnvironmentExtension.hpp"
#include "Acts/Propagator/MaterialInteractor.hpp"
#include "Acts/Propagator/Navigator.hpp"
#include "Acts/Propagator/Propagator.hpp"
#include "Acts/Propagator/StandardAborters.hpp"
#include "Acts/Propagator/detail/SteppingLogger.hpp"
#include "Acts/Surfaces/PerigeeSurface.hpp"
#include "Acts/Utilities/Logger.hpp"

// Kalman Filter

// #include "Acts/EventData/Measurement.hpp"
#include "Acts/EventData/MultiTrajectory.hpp"
#include "Acts/EventData/MultiTrajectoryHelpers.hpp"
#include "Acts/EventData/VectorTrackContainer.hpp"
#include "Acts/Geometry/GeometryIdentifier.hpp"
#include "Acts/TrackFinding/CombinatorialKalmanFilter.hpp"
#include "Acts/TrackFinding/MeasurementSelector.hpp"
#include "Acts/TrackFitting/GainMatrixSmoother.hpp"
#include "Acts/TrackFitting/GainMatrixUpdater.hpp"
#include "Acts/Utilities/CalibrationContext.hpp"

//--- Refit with backward propagation ---//
#include "Acts/TrackFitting/KalmanFitter.hpp"

// GSF
#include "Acts/Propagator/MultiEigenStepperLoop.hpp"
#include "Acts/TrackFitting/BetheHeitlerApprox.hpp"
#include "Acts/TrackFitting/GaussianSumFitter.hpp"
#include "Acts/TrackFitting/GsfMixtureReduction.hpp"

//--- Tracking ---//
#include "Tracking/Event/Measurement.h"
#include "Tracking/Event/Track.h"
#include "Tracking/Reco/TrackExtrapolatorTool.h"
#include "Tracking/Sim/IndexSourceLink.h"
#include "Tracking/Sim/MeasurementCalibrator.h"
#include "Tracking/Sim/TrackingUtils.h"

//--- Interpolated magnetic field ---//
#include "Tracking/Sim/BFieldXYZUtils.h"

using ActionList =
    Acts::ActionList<Acts::detail::SteppingLogger, Acts::MaterialInteractor>;
using AbortList = Acts::AbortList<Acts::EndOfWorldReached>;

// using GsfPropagator = Acts::Propagator<
//                         Acts::MultiEigenStepperLoop<
//                           Acts::StepperExtensionList<
//                            Acts::detail::GenericDefaultExtension<double> >,
//                           Acts::WeightedComponentReducerLoop,
//                           Acts::detail::VoidAuctioneer>,
//                         Acts::Navigator>;

using MultiStepper = Acts::MultiEigenStepperLoop<>;
using Propagator = Acts::Propagator<Acts::EigenStepper<>, Acts::Navigator>;
using GsfPropagator = Acts::Propagator<MultiStepper, Acts::Navigator>;
using BetheHeitlerApprox = Acts::AtlasBetheHeitlerApprox<6, 5>;

namespace tracking {
namespace reco {

class GSFProcessor final : public TrackingGeometryUser {
 public:
  /**
   * Constructor.
   *
   * @param name The name of the instance of this object.
   * @param process The process running this producer.
   */
  GSFProcessor(const std::string &name, framework::Process &process);

  /// Destructor
  virtual ~GSFProcessor() = default;

  /**
   *
   */
  void onProcessStart() override;

  /**
   * onNewRun is the first function called for each processor
   * *after* the conditions are fully configured and accessible.
   * This is where you could create single-processors, multi-event
   * calculation objects.
   */
  void onNewRun(const ldmx::RunHeader &rh) override;

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
  // Forms the layer to acts map
  // auto makeLayerSurfacesMap(std::shared_ptr<const Acts::TrackingGeometry>
  // trackingGeometry) const -> std::unordered_map<unsigned int, const
  // Acts::Surface*>;

  // Make geoid -> source link map Measurements
  // auto makeGeoIdSourceLinkMap(
  //     const geo::TrackersTrackingGeometry& tg,
  //     const std::vector<ldmx::Measurement > &ldmxsps) ->
  //     std::unordered_multimap<Acts::GeometryIdentifier,
  //     ActsExamples::IndexSourceLink>;

  // If we want to dump the tracking geometry
  // bool dumpobj_{false};
  // int pionstates_{10};
  // int nevents_{0};
  // Processing time counter
  // double processing_time_{0.};

  /// Time profiling data for performance analysis
  std::map<std::string, double> profiling_map_;

  // refitting of tracks
  // bool kf_refit_{false};
  // bool gsf_refit_{false};

  /// Enable verbose debug output logging
  bool debug_{false};

  /// Random number generator for smearing operations
  std::default_random_engine generator_;

  /// Normal distribution for smearing measurements
  std::shared_ptr<std::normal_distribution<float>> normal_;

  // Constant BField
  // double bfield_{0};
  // Use constant bfield
  // bool const_b_field_{true};

  // Remove stereo measurements
  // bool remove_stereo_{false};

  // Use 2d measurements instead of 1D
  // bool use1Dmeasurements_{true};

  // Minimum number of hits on tracks
  // int min_hits_{7};

  /// Location to extrapolate tracks to (x, y, z in mm)
  std::vector<double> extrapolate_location_{0., 0., 0.};
  // bool use_seed_perigee_{false};

  /// Collection name for input measurements
  std::string measurement_collection_{"TaggerMeasurements"};

  // double outlier_pval_{3.84};

  /// Collection name for output GSF-refitted tracks
  std::string out_trk_collection_{"GSFTracks"};

  // Select the hits using TrackID and pdg_id__

  // int track_id_{-1};
  // int pdg_id_{11};

  // Mass for the propagator hypothesis in MeV
  // double mass_{0.511};

  /// Collection name for seed tracks (currently unused)
  std::string seed_coll_name_{"seedTracks"};

  /// Gaussian Sum Fitter instance for track refitting
  std::unique_ptr<const Acts::GaussianSumFitter<
      GsfPropagator, BetheHeitlerApprox, Acts::VectorMultiTrajectory>>
      gsf_;

  /// Collection name for input tracks to be refit
  std::string track_collection_{"TaggerTracks"};

  /// Collection name for measurements associated with tracks
  std::string meas_collection_{"DigiTaggerSimHits"};

  /// Pass name for track collection in event
  std::string track_passname_;

  /// Pass name for measurement collection in event
  std::string meas_passname_;

  /// Pass name qualifier for track collection event key
  std::string track_collection_event_passname_;

  /// Pass name qualifier for measurement collection event key
  std::string meas_collection_event_passname_;

  /// Maximum number of mixture components in GSF fit
  size_t max_components_{4};

  /// Abort fit if any error occurs (strict error handling)
  bool abort_on_error_{false};

  /// Disable all material interactions during propagation
  bool disable_all_material_handling_{false};

  /// Weight threshold below which mixture components are dropped
  double weight_cutoff_{1.0e-4};

  /// Step size for track propagation in mm
  double propagator_step_size_{200.};

  /// Maximum number of propagation steps before aborting
  int propagator_max_steps_{1000};

  /// Path to magnetic field map file
  std::string field_map_{""};

  /// Use perigee parameterization for tracks
  bool use_perigee_{false};

  // Keep track on which system this processor is running on
  bool tagger_tracking_{true};

  /// Propagator for track extrapolation using eigen stepper
  std::unique_ptr<const Propagator> propagator_;

  /// Layer ID to ACTS Surface mapping for hit surface lookup
  std::unordered_map<unsigned int, const Acts::Surface *> layer_surface_map_;

  // Track Extrapolator Tool
  std::shared_ptr<tracking::reco::TrackExtrapolatorTool<Propagator>>
      trk_extrap_;

  /// Beam origin surface at z=-700 mm (tagger track initialization)
  std::shared_ptr<Acts::Surface> beam_origin_surface_;

  /// Target surface at z=0 mm (recoil track initialization, perigee output)
  std::shared_ptr<Acts::Surface> target_surface_;

  /// ECAL surface at z=240.5 mm (ECAL scoring plane for recoil tracking)
  std::shared_ptr<Acts::Surface> ecal_surface_;

};  // GSFProcessor

}  // namespace reco
}  // namespace tracking

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
#include "Acts/MagneticField/ConstantBField.hpp"
#include "Acts/Propagator/AbortList.hpp"
#include "Acts/Propagator/ActionList.hpp"
#include "Acts/Propagator/DenseEnvironmentExtension.hpp"
#include "Acts/Propagator/EigenStepper.hpp"
#include "Acts/Propagator/MaterialInteractor.hpp"
#include "Acts/Propagator/Navigator.hpp"
#include "Acts/Propagator/Propagator.hpp"
#include "Acts/Propagator/StandardAborters.hpp"
#include "Acts/Propagator/detail/SteppingLogger.hpp"
#include "Acts/Surfaces/PerigeeSurface.hpp"
#include "Acts/Utilities/Logger.hpp"

// Kalman Filter

//#include "Acts/EventData/Measurement.hpp"
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
using DirectPropagator = Acts::Propagator<MultiStepper, Acts::DirectNavigator>;
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

  // time profiling
  std::map<std::string, double> profiling_map_;

  // refitting of tracks
  // bool kf_refit_{false};
  // bool gsf_refit_{false};

  bool debug_{false};

  //--- Smearing ---//

  std::default_random_engine generator_;
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

  // The extrapolation surface
  // bool use_extrapolate_location_{true};
  std::vector<double> extrapolate_location_{0., 0., 0.};
  // bool use_seed_perigee_{false};

  // The measurement collection to use for track reconstruction
  std::string measurement_collection_{"TaggerMeasurements"};

  // double outlier_pval_{3.84};

  // The output track collection
  std::string out_trk_collection_{"GSFTracks"};

  // Select the hits using TrackID and pdg_id__

  // int track_id_{-1};
  // int pdg_id_{11};

  // Mass for the propagator hypothesis in MeV
  // double mass_{0.511};

  // The seed track collection
  std::string seed_coll_name_{"seedTracks"};

  // The GSF Fitter
  std::unique_ptr<const Acts::GaussianSumFitter<
      GsfPropagator, BetheHeitlerApprox, Acts::VectorMultiTrajectory>>
      gsf_;

  // The Direct GSF Fitter
  std::unique_ptr<const Acts::GaussianSumFitter<
      DirectPropagator, BetheHeitlerApprox, Acts::VectorMultiTrajectory>>
      gsf_direct_;

  // Configuration

  std::string trackCollection_{"TaggerTracks"};
  std::string measCollection_{"DigiTaggerSimHits"};

  size_t maxComponents_{4};
  bool abortOnError_{false};
  bool disableAllMaterialHandling_{false};
  double weightCutoff_{1.0e-4};

  double propagator_step_size_{200.};  // mm
  int propagator_maxSteps_{1000};
  std::string field_map_{""};

  bool usePerigee_{false};
  // bool usePlaneSurface_{false};

  // Keep track on which system this processor is running on
  bool taggerTracking_{true};

  // The Propagators
  std::unique_ptr<const Propagator> propagator_;

  // The GSF Fitter

  // This could be a vector
  // The mapping between layers and Acts::Surface
  std::unordered_map<unsigned int, const Acts::Surface *> layer_surface_map_;

  // Track Extrapolator Tool
  std::shared_ptr<tracking::reco::TrackExtrapolatorTool<Propagator>>
      trk_extrap_;

};  // GSFProcessor

}  // namespace reco
}  // namespace tracking

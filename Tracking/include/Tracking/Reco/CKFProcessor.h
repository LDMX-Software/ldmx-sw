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
//#include "Acts/TrackFitting/GaussianSumFitter.hpp"
#include "Acts/Propagator/MultiEigenStepperLoop.hpp"

//--- Tracking ---//
#include "Tracking/Event/Measurement.h"
#include "Tracking/Event/Track.h"
#include "Tracking/Reco/TrackExtrapolatorTool.h"
#include "Tracking/Sim/IndexSourceLink.h"
#include "Tracking/Sim/MeasurementCalibrator.h"
#include "Tracking/Sim/TrackingUtils.h"

//--- Interpolated magnetic field ---//
#include "Tracking/Sim/BFieldXYZUtils.h"
// mg Aug 2024 not sure if these are needed...
using Updater = Acts::GainMatrixUpdater;
using Smoother = Acts::GainMatrixSmoother;

using ActionList =
    Acts::ActionList<Acts::detail::SteppingLogger, Acts::MaterialInteractor>;
using AbortList = Acts::AbortList<Acts::EndOfWorldReached>;

using CkfPropagator = Acts::Propagator<Acts::EigenStepper<>, Acts::Navigator>;
using TrackContainer = Acts::TrackContainer<Acts::VectorTrackContainer,
                                            Acts::VectorMultiTrajectory>;

namespace tracking {
namespace reco {

class CKFProcessor final : public TrackingGeometryUser {
 public:
  /**
   * Constructor.
   *
   * @param name The name of the instance of this object.
   * @param process The process running this producer.
   */
  CKFProcessor(const std::string &name, framework::Process &process);

  /// Destructor
  ~CKFProcessor();

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
  // Make geoid -> source link map Measurements
  auto makeGeoIdSourceLinkMap(const geo::TrackersTrackingGeometry &tg,
                              const std::vector<ldmx::Measurement> &ldmxsps)
      -> std::unordered_multimap<Acts::GeometryIdentifier,
                                 ActsExamples::IndexSourceLink>;

  // If we want to dump the tracking geometry
  bool dumpobj_{false};

  int pionstates_{10};

  int nevents_{0};

  // Processing time counter
  double processing_time_{0.};

  // time profiling
  std::map<std::string, double> profiling_map_;

  bool debug_acts_{false};

  std::shared_ptr<Acts::PlaneSurface> target_surface;
  Acts::RotationMatrix3 surf_rotation;
  // Constant BField
  double bfield_{0};
  // Use constant bfield
  bool const_b_field_{true};

  // Remove stereo measurements
  bool remove_stereo_{false};

  // Use 2d measurements instead of 1D
  bool use1Dmeasurements_{true};

  // Minimum number of hits on tracks
  int min_hits_{7};

  // Stepping size (in mm)
  double propagator_step_size_{200.};
  int propagator_maxSteps_{1000};

  // The extrapolation surface
  bool use_extrapolate_location_{true};
  std::vector<double> extrapolate_location_{0., 0., 0.};
  bool use_seed_perigee_{false};

  // The measurement collection to use for track reconstruction
  std::string measurement_collection_{"TaggerMeasurements"};

  // Outlier removal pvalue
  // The Chi2Cut is applied at filtering stage.
  // 1DOF pvalues: 0.1 = 2.706 0.05 = 3.841 0.025 = 5.024 0.01 = 6.635 0.005
  // = 7.879 The probability to reject a good measurement is pvalue The
  // probability to reject an outlier is given in NIM A262 (1987) 444-450

  double outlier_pval_{3.84};

  // The output track collection
  std::string out_trk_collection_{"Tracks"};

  // Mass for the propagator hypothesis in MeV
  double mass_{0.511};

  // The seed track collection
  std::string seed_coll_name_{"seedTracks"};

  // The interpolated bfield
  std::string field_map_{""};

  // The Propagator
  std::unique_ptr<const CkfPropagator> propagator_;

  // The CKF
  std::unique_ptr<
      const Acts::CombinatorialKalmanFilter<CkfPropagator, TrackContainer>>
      ckf_;

  // Track Extrapolator Tool
  std::shared_ptr<tracking::reco::TrackExtrapolatorTool<CkfPropagator>>
      trk_extrap_;

  /// n seeds and n tracks
  int nseeds_{0};
  int ntracks_{0};
  int eventnr_{0};

  // BField Systematics
  std::vector<double> map_offset_{
      0.,
      0.,
      0.,
  };

  // Keep track on which system this processor is running on
  bool taggerTracking_{true};

};  // CKFProcessor

}  // namespace reco
}  // namespace tracking

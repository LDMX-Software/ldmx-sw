#ifndef TRACKING_RECO_CKFPROCESSOR_H_
#define TRACKING_RECO_CKFPROCESSOR_H_

//--- Framework ---//
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"
#include "Framework/RandomNumberSeedService.h"

//--- Tracking I/O---//
#include "Tracking/Sim/PropagatorStepWriter.h"

//--- C++ ---//
#include <random>
#include <memory>

//--- LDMX ---//
#include "Tracking/Reco/TrackersTrackingGeometry.h"

//--- ACTS ---//

//Utils and Definitions
#include "Acts/Utilities/Logger.hpp"
#include "Acts/Definitions/Units.hpp"
#include "Acts/Definitions/Common.hpp"
#include "Acts/Definitions/TrackParametrization.hpp"
#include "Acts/EventData/TrackParameters.hpp"
#include "Acts/EventData/detail/TransformationFreeToBound.hpp"

//geometry
#include "Acts/Geometry/GeometryContext.hpp"

//magfield
#include "Acts/MagneticField/MagneticFieldContext.hpp"
#include "Acts/MagneticField/MagneticFieldProvider.hpp"

//geometry
#include <Acts/Geometry/TrackingGeometry.hpp>


//propagation testing
#include "Acts/Propagator/Navigator.hpp"
#include "Acts/Propagator/Propagator.hpp"
#include "Acts/Propagator/EigenStepper.hpp"
#include "Acts/Propagator/StandardAborters.hpp"
#include "Acts/Utilities/Logger.hpp"
#include "Acts/MagneticField/ConstantBField.hpp"
#include "Acts/Propagator/MaterialInteractor.hpp"
#include "Acts/Propagator/AbortList.hpp"
#include "Acts/Propagator/ActionList.hpp"
#include "Acts/Propagator/DenseEnvironmentExtension.hpp"
#include "Acts/Propagator/detail/SteppingLogger.hpp"
#include "Acts/Surfaces/PerigeeSurface.hpp"


//Kalman Filter

//#include "Acts/EventData/Measurement.hpp"
#include "Acts/Geometry/GeometryIdentifier.hpp" 
#include "Acts/TrackFitting/GainMatrixSmoother.hpp"
#include "Acts/TrackFitting/GainMatrixUpdater.hpp"
#include "Acts/Utilities/CalibrationContext.hpp"
#include "Acts/TrackFinding/MeasurementSelector.hpp"
#include "Acts/TrackFinding/CombinatorialKalmanFilter.hpp"
#include "Acts/EventData/MultiTrajectory.hpp"
#include "Acts/EventData/MultiTrajectoryHelpers.hpp"
#include "Acts/EventData/VectorTrackContainer.hpp"


//--- Refit with backward propagation ---//
#include "Acts/TrackFitting/KalmanFitter.hpp"


//GSF
//#include "Acts/TrackFitting/GaussianSumFitter.hpp"
#include "Acts/Propagator/MultiEigenStepperLoop.hpp"

//--- Tracking ---//
#include "Tracking/Sim/TrackingUtils.h"
#include "Tracking/Sim/IndexSourceLink.h"
#include "Tracking/Sim/MeasurementCalibrator.h"
#include "Tracking/Event/Track.h"
#include "Tracking/Event/Measurement.h"


//--- Interpolated magnetic field ---//
#include "Tracking/Sim/BFieldXYZUtils.h"

using ActionList = Acts::ActionList<Acts::detail::SteppingLogger, Acts::MaterialInteractor>;
using AbortList = Acts::AbortList<Acts::EndOfWorldReached>;

using CkfPropagator = Acts::Propagator<Acts::EigenStepper<>, Acts::Navigator>;
using GsfPropagator = Acts::Propagator<
                        Acts::MultiEigenStepperLoop<
                          Acts::StepperExtensionList<
                            Acts::detail::GenericDefaultExtension<double> >,
                          Acts::WeightedComponentReducerLoop,
                          Acts::detail::VoidAuctioneer>,
                        Acts::Navigator>;

//?!
//using PropagatorOptions =
//    Acts::DenseStepperPropagatorOptions<ActionList, AbortList>;

namespace tracking {
namespace reco {

class CKFProcessor final : public framework::Producer {

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


  //TODO move it away

  void propagateENstates(framework::Event &event,std::string inputFile, std::string outFile);

  
  //Forms the layer to acts map
  auto makeLayerSurfacesMap(std::shared_ptr<const Acts::TrackingGeometry> trackingGeometry) const -> std::unordered_map<unsigned int, const Acts::Surface*>;

  //Make geoid -> source link map Measurements
  auto makeGeoIdSourceLinkMap(const std::vector<ldmx::Measurement > &ldmxsps) -> std::unordered_multimap<Acts::GeometryIdentifier, ActsExamples::IndexSourceLink>;
    
    
  //Test the magnetic field

  void testField(const std::shared_ptr<Acts::MagneticFieldProvider> bField,
                 const Acts::Vector3& eval_pos) const;
  
  // Make a simple event display
  void writeEvent(framework::Event &event,
		  const Acts::BoundTrackParameters& perigeeParameters,
		  const Acts::MultiTrajectory<Acts::VectorMultiTrajectory>& mj,
                  const int& trackTip,
                  const std::vector<ldmx::Measurement> meas);

  
  /// The tracking geometry
  std::shared_ptr<tracking::reco::TrackersTrackingGeometry> ldmx_tg;
  
  /// The contexts
  Acts::GeometryContext gctx_;
  Acts::MagneticFieldContext bctx_;
  Acts::CalibrationContext cctx_;

  /// The path to the GDML description of the detector
  std::string detector_{""};

  //If we want to dump the tracking geometry
  bool dumpobj_ {false};

  int pionstates_{10};

  int nevents_{0};

  //Processing time counter
  double processing_time_{0.};

  //time profiling
  std::map<std::string, double> profiling_map_;
  
  //refitting of tracks
  bool kf_refit_{false};
  bool gsf_refit_{false};

  bool debug_{false};


  //--- Smearing ---//

  std::default_random_engine generator_;
  std::shared_ptr<std::normal_distribution<float>> normal_;

  //Constant BField
  double bfield_{0};
  //Use constant bfield
  bool const_b_field_{true};

  //Remove stereo measurements
  bool remove_stereo_{false};

  //Use 2d measurements instead of 1D
  bool use1Dmeasurements_{true};
  
  //Minimum number of hits on tracks
  int min_hits_{7};
  
  //Stepping size (in mm)
  double propagator_step_size_{200.};
  int propagator_maxSteps_{1000};
  
  //The extrapolation surface
  bool use_extrapolate_location_{true};
  std::vector<double> extrapolate_location_{0.,0.,0.};
  bool use_seed_perigee_{false};
  
  //The measurement collection to use for track reconstruction
  std::string measurement_collection_{"TaggerMeasurements"};

  // Outlier removal pvalue
  // The Chi2Cut is applied at filtering stage.
  // 1DOF pvalues: 0.1 = 2.706 0.05 = 3.841 0.025 = 5.024 0.01 = 6.635 0.005 = 7.879
  // The probability to reject a good measurement is pvalue
  // The probability to reject an outlier is given in NIM A262 (1987) 444-450

  double outlier_pval_{3.84};
  
  //The output track collection
  std::string out_trk_collection_{"Tracks"};

  //Select the hits using TrackID and pdg_id__
  
  int track_id_{-1};
  int pdg_id_{11};

  //Mass for the propagator hypothesis in MeV
  double mass_{0.511};
  

  //The seed track collection
  std::string seed_coll_name_{"seedTracks"};

  //The interpolated bfield
  std::string field_map_{""};

  //The Propagators
  std::unique_ptr<const CkfPropagator> propagator_;

  //The CKF
  std::unique_ptr<const Acts::CombinatorialKalmanFilter<CkfPropagator,Acts::VectorMultiTrajectory>> ckf_;

  //The KF
  std::unique_ptr<const Acts::KalmanFitter<CkfPropagator,Acts::VectorMultiTrajectory>> kf_;

  //The GSF Fitter
  //std::unique_ptr<const Acts::GaussianSumFitter<GsfPropagator>> gsf_;
  
  //The propagator steps writer
  std::unique_ptr<tracking::sim::PropagatorStepWriter> writer_;

  //Outname of the propagator test
  std::string steps_outfile_path_{""};

  //This could be a vector
  //The mapping between layers and Acts::Surface
  std::unordered_map<unsigned int, const Acts::Surface*> layer_surface_map_;
  
  /// n seeds and n tracks
  int nseeds_{0};
  int ntracks_{0};

  int eventnr_{0};
  
}; // CKFProcessor
    

} // namespace reco
} // namespace tracking

#endif // CKFProcessor

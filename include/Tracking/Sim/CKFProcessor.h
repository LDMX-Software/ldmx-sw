#ifndef TRACKING_SIM_CKFPROCESSOR_H_
#define TRACKING_SIM_CKFPROCESSOR_H_

//--- Framework ---//
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"
#include "Framework/RandomNumberSeedService.h"

//---  DD4hep ---//
#include "DD4hep/Detector.h"

//--- ROOT ---//
#include "TGeoMatrix.h"

//--- Tracking I/O---//
#include "Tracking/Sim/PropagatorStepWriter.h"

//--- C++ ---//
#include <random>
#include <memory>

//--- LDMX ---//
#include "Tracking/Reco/LdmxTrackingGeometry.h"

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

//Step 1 - gather the measurements
//#include "Tracking/Sim/LdmxMeasurement.h" <-Not needed

//#include "Acts/EventData/Measurement.hpp"
#include "Acts/Geometry/GeometryIdentifier.hpp" 
#include "Acts/TrackFitting/GainMatrixSmoother.hpp"
#include "Acts/TrackFitting/GainMatrixUpdater.hpp"
#include "Acts/Utilities/CalibrationContext.hpp"
#include "Acts/TrackFinding/MeasurementSelector.hpp"
#include "Acts/TrackFinding/CombinatorialKalmanFilter.hpp"
#include "Acts/EventData/MultiTrajectory.hpp"
#include "Acts/EventData/MultiTrajectoryHelpers.hpp"

//--- Refit with backward propagation ---//
#include "Acts/TrackFitting/KalmanFitter.hpp"


//GSF
#include "Acts/TrackFitting/GaussianSumFitter.hpp"
#include "Acts/Propagator/MultiEigenStepperLoop.hpp"

//--- Tracking ---//
#include "Tracking/Sim/TrackingUtils.h"
#include "Tracking/Sim/IndexSourceLink.h"
#include "Tracking/Sim/LdmxSourceLinkAccessor.h"
#include "Tracking/Sim/MeasurementCalibrator.h"
#include "Tracking/Event/Track.h"


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
namespace sim {

class CKFProcessor : public framework::Producer {

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
  void onProcessStart() final override;

  /**
   *
   */
  void onProcessEnd() final override;

  /**
   * Configure the processor using the given user specified parameters.
   *
   * @param parameters Set of parameters used to configure this processor.
   */
  void configure(framework::config::Parameters &parameters) final override;

  /**
   * Run the processor
   *
   * @param event The event to process.
   */
  void produce(framework::Event &event);

  //Forms the layer to acts map
  void makeLayerSurfacesMap(std::shared_ptr<const Acts::TrackingGeometry> trackingGeometry);

  
  //Test the measurement calibrator (TODO::move it somewhere else)

  void testMeasurmentCalibrator(const LdmxMeasurementCalibrator& calibrator,
                                const std::unordered_map<Acts::GeometryIdentifier, std::vector< ActsExamples::IndexSourceLink> > & map);

  //Test the magnetic field

  void testField(const std::shared_ptr<Acts::MagneticFieldProvider> bField,
                 const Acts::Vector3& eval_pos);
  
  // Make a simple event display
  bool WriteEvent(framework::Event &event,
		  const Acts::BoundTrackParameters& perigeeParameters,
		  const Acts::MultiTrajectory& mj,
                  const int& trackTip,
                  const std::vector<ldmx::LdmxSpacePoint*> ldmxsps);
  
 private:
  /// The detector
  dd4hep::Detector* detector_{nullptr};
  
  /// The tracking geometry
  std::shared_ptr<tracking::reco::LdmxTrackingGeometry> ldmx_tg;
  
  /// The contexts
  Acts::GeometryContext gctx_;
  Acts::MagneticFieldContext bctx_;
  Acts::CalibrationContext cctx_;

  //If we want to dump the tracking geometry
  int dumpobj_ {0};

  bool debug_{false};
  int nevents_{0};

  //Processing time counter
  double processing_time_{0.};

  //time profiling
  std::map<std::string, double> profiling_map_;
  
  //refitting of tracks
  bool kfRefit_{false};
  bool gsfRefit_{false};
  
  //--- Smearing ---//

  std::default_random_engine generator_;
  std::shared_ptr<std::normal_distribution<float>> normal_;

  //Constant BField
  double bfield_{0};
  //Use constant bfield
  bool const_b_field_{true};

  //Remove stereo measurements
  bool removeStereo_{false};

  //Use 2d measurements instead of 1D
  bool use1Dmeasurements_{true};
  
  //Minimum number of hits on tracks
  int minHits_{7};
  
  //Stepping size (in mm)
  double propagator_step_size_{200.};
  int propagator_maxSteps_{1000};

  //The perigee location used for the initial propagator states generation
  std::vector<double> perigee_location_{0.,0.,0.};

  //The extrapolation surface
  bool use_extrapolate_location_{true};
  std::vector<double> extrapolate_location_{0.,0.,0.};
  bool use_seed_perigee_{false};
  
  //The hit collection to use for track reconstruction
  std::string hit_collection_{"TaggerSimHits"};
  
  //The output track collection
  std::string out_trk_collection_{"Tracks"};

  //Select the hits using TrackID and pdgID_
  
  int trackID_{-1};
  int pdgID_{11};

  //Mass for the propagator hypothesis in MeV
  double mass_{0.511};
  

  //The seed track collection
  std::string seed_coll_name_{"seedTracks"};

  //The interpolated bfield
  std::string bfieldMap_;

  //The Propagators
  std::unique_ptr<const CkfPropagator> propagator_;

  //The CKF
  std::unique_ptr<const Acts::CombinatorialKalmanFilter<CkfPropagator>> ckf_;

  //The KF
  std::unique_ptr<const Acts::KalmanFitter<CkfPropagator>> kf_;

  //The GSF Fitter
  std::unique_ptr<const Acts::GaussianSumFitter<GsfPropagator>> gsf_;
  
  //The propagator steps writer
  std::unique_ptr<PropagatorStepWriter> writer_;

  //Outname of the propagator test
  std::string steps_outfile_path_{""};

  //This could be a vector
  //The mapping between layers and Acts::Surface
  std::unordered_map<unsigned int, const Acts::Surface*> layer_surface_map_;
  
  //Some histograms
  std::unique_ptr<TH1F> histo_p_;
  std::unique_ptr<TH1F> histo_d0_;
  std::unique_ptr<TH1F> histo_z0_;
  std::unique_ptr<TH1F> histo_phi_;
  std::unique_ptr<TH1F> histo_theta_;
  std::unique_ptr<TH1F> histo_qop_;

  std::unique_ptr<TH1F> histo_p_pull_;
  std::unique_ptr<TH1F> histo_d0_pull_;
  std::unique_ptr<TH1F> histo_z0_pull_;
  std::unique_ptr<TH1F> histo_phi_pull_;
  std::unique_ptr<TH1F> histo_theta_pull_;
  std::unique_ptr<TH1F> histo_qop_pull_;
  
  std::unique_ptr<TH1F> h_p_;
  std::unique_ptr<TH1F> h_d0_;
  std::unique_ptr<TH1F> h_z0_;
  std::unique_ptr<TH1F> h_phi_;
  std::unique_ptr<TH1F> h_theta_;
  std::unique_ptr<TH1F> h_qop_;
  std::unique_ptr<TH1F> h_nHits_;

  std::unique_ptr<TH1F> h_p_err_;
  std::unique_ptr<TH1F> h_d0_err_;
  std::unique_ptr<TH1F> h_z0_err_;
  std::unique_ptr<TH1F> h_phi_err_;
  std::unique_ptr<TH1F> h_theta_err_;
  std::unique_ptr<TH1F> h_qop_err_;

  std::unique_ptr<TH1F> h_p_refit_;
  std::unique_ptr<TH1F> h_d0_refit_;
  std::unique_ptr<TH1F> h_z0_refit_;
  std::unique_ptr<TH1F> h_phi_refit_;
  std::unique_ptr<TH1F> h_theta_refit_;
  std::unique_ptr<TH1F> h_nHits_refit_;

  std::unique_ptr<TH1F> h_p_gsf_refit_;
  std::unique_ptr<TH1F> h_d0_gsf_refit_;
  std::unique_ptr<TH1F> h_z0_gsf_refit_;
  std::unique_ptr<TH1F> h_phi_gsf_refit_;
  std::unique_ptr<TH1F> h_theta_gsf_refit_;
  std::unique_ptr<TH1F> h_p_gsf_refit_res_;
  std::unique_ptr<TH1F> h_qop_gsf_refit_res_;
  
  std::unique_ptr<TH1F> h_p_truth_;
  std::unique_ptr<TH1F> h_d0_truth_;
  std::unique_ptr<TH1F> h_z0_truth_;
  std::unique_ptr<TH1F> h_phi_truth_;
  std::unique_ptr<TH1F> h_theta_truth_;
  std::unique_ptr<TH1F> h_qop_truth_;

  std::unique_ptr<TH2F> h_tgt_scoring_x_y_;
  std::unique_ptr<TH1F> h_tgt_scoring_z_;

  /// do smearing
  bool do_smearing_{false};
  
  /// u-direction sigma
  double sigma_u_{0};

  /// v-direction sigma
  double sigma_v_{0};

  /// n seeds and n tracks
  int nseeds_{0};
  int ntracks_{0};
  
}; // CKFProcessor
    

} // namespace sim
} // namespace tracking

#endif // CKFProcessor

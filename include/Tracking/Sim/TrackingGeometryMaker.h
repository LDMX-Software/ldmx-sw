#ifndef TRACKING_SIM_TRACKINGGEOMAKER_H_
#define TRACKING_SIM_TRACKINGGEOMAKER_H_

//--- Framework ---//
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"

//---  DD4hep ---//
#include "DD4hep/Detector.h"

//--- ROOT ---//
#include "TGeoMatrix.h"

//--- Tracking I/O---//
#include "Tracking/Sim/PropagatorStepWriter.h"

//--- C++ ---//
#include <random>

//--- ACTS ---//

//Utils and Definitions
#include "Acts/Utilities/Logger.hpp"
#include "Acts/Definitions/Units.hpp"
#include "Acts/Definitions/Common.hpp"
#include "Acts/Definitions/TrackParametrization.hpp"
#include "Acts/EventData/TrackParameters.hpp"
#include "Acts/EventData/detail/TransformationFreeToBound.hpp"

//dd4hep
#include "Acts/Plugins/DD4hep/ActsExtension.hpp"
#include "Acts/Plugins/DD4hep/DD4hepLayerBuilder.hpp"
#include "Acts/Plugins/DD4hep/DD4hepDetectorElement.hpp"

//geometry
#include "Acts/Geometry/CuboidVolumeBuilder.hpp"
#include "Acts/Geometry/GeometryContext.hpp"

//magfield
#include "Acts/MagneticField/MagneticFieldContext.hpp"
#include "Acts/MagneticField/MagneticFieldProvider.hpp"

#include "Acts/Geometry/TrackingVolume.hpp"
#include "Acts/Geometry/TrackingGeometryBuilder.hpp"
#include <Acts/Geometry/TrackingGeometry.hpp>
#include "Acts/Surfaces/RectangleBounds.hpp"

///Visualization
#include <Acts/Visualization/ObjVisualization3D.hpp>
#include <Acts/Visualization/GeometryView3D.hpp>
#include <Acts/Visualization/ViewConfig.hpp>

//Material
//This should be changed in the new version
//#include "Acts/Material/MaterialProperties.hpp"
#include "Acts/Material/Material.hpp"
#include "Acts/Material/MaterialSlab.hpp"
#include "Acts/Material/HomogeneousSurfaceMaterial.hpp"
#include "Acts/Material/HomogeneousVolumeMaterial.hpp"


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
#include "Tracking/Sim/MeasurementCalibrator.h"

//--- Tracking ---//
#include "Tracking/Sim/TrackingUtils.h"
#include "Tracking/Sim/IndexSourceLink.h"
#include "Tracking/Sim/LdmxSourceLinkAccessor.h"
#include "Acts/EventData/MultiTrajectory.hpp"

//--- Interpolated magnetic field ---//
#include "Tracking/Sim/BFieldXYZUtils.h"

using ActionList = Acts::ActionList<Acts::detail::SteppingLogger, Acts::MaterialInteractor>;
using AbortList = Acts::AbortList<Acts::EndOfWorldReached>;
using Propagator = Acts::Propagator<Acts::EigenStepper<>, Acts::Navigator>;


using PropagatorOptions =
    Acts::DenseStepperPropagatorOptions<ActionList, AbortList>;

namespace tracking {
namespace sim {

class TrackingGeometryMaker : public framework::Producer {

 public:
  /**
   * Constructor.
   *
   * @param name The name of the instance of this object.
   * @param process The process running this producer.
   */
  TrackingGeometryMaker(const std::string &name, framework::Process &process);

  /// Destructor
  ~TrackingGeometryMaker();
  
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

  Acts::CuboidVolumeBuilder::VolumeConfig volumeBuilder_gdml(Acts::Logging::Level logLevel);
  
  Acts::CuboidVolumeBuilder::VolumeConfig  volumeBuilder_dd4hep(dd4hep::DetElement& subdetector,Acts::Logging::Level logLevel);
    
  void collectSubDetectors_dd4hep(dd4hep::DetElement& detElement,
                                  std::vector<dd4hep::DetElement>& subdetectors);
  void collectSensors_dd4hep(dd4hep::DetElement& detElement,
                             std::vector<dd4hep::DetElement>& sensors);

  void collectModules_dd4hep(dd4hep::DetElement& detElement,
                             std::vector<dd4hep::DetElement>& modules);
   

  //This should go and we should use ACTS methods. But they are private for the moment.
  void resolveSensitive(
      const dd4hep::DetElement& detElement,
      std::vector<std::shared_ptr<const Acts::Surface>>& surfaces,bool force) const;

  std::shared_ptr<const Acts::Surface>
  createSensitiveSurface(
      const dd4hep::DetElement& detElement) const;
  
  Acts::Transform3 convertTransform(const TGeoMatrix* tGeoTrans) const;
  
  //Get the sensitive surfaces out of the tracking geometry
  void getSurfaces(std::vector<const Acts::Surface*>& surfaces,
                   std::shared_ptr<const Acts::TrackingGeometry> trackingGeometry);

  //Forms the layer to acts map
  void makeLayerSurfacesMap(std::shared_ptr<const Acts::TrackingGeometry> trackingGeometry);

  
  template <typename source_link_accessor_t> 
  void testAccessor(source_link_accessor_t accessor,
                    const typename source_link_accessor_t::Container& container,
                    const Acts::GeometryIdentifier& id);


  template <typename T>
  std::string type_name();
  
  //Test the measurement calibrator (TODO::move it somewhere else)

  void testMeasurmentCalibrator(const LdmxMeasurementCalibrator& calibrator,
                                const std::unordered_map<Acts::GeometryIdentifier, std::vector< ActsExamples::IndexSourceLink> > & map);

  //Test the magnetic field

  void testField(const std::shared_ptr<Acts::MagneticFieldProvider> bField);
  
 private:
  /// The detector
  dd4hep::Detector* detector_{nullptr};
  Acts::GeometryContext gctx_;
  Acts::MagneticFieldContext bctx_;
  Acts::CalibrationContext cctx_;

  //If we want to dump the tracking geometry
  int dumpobj_ {0};

  bool debug_{false};
  int n_events_{0};

  //--- Propagator Tests ---//

  //Random number generator
  int ntests_{0};
  std::vector<double> phi_range_,theta_range_;
  std::default_random_engine generator_;
  std::shared_ptr<std::uniform_real_distribution<double> > uniform_phi_;
  std::shared_ptr<std::uniform_real_distribution<double> > uniform_theta_;
  std::shared_ptr<std::normal_distribution<float>> normal_;

  //Constant BField
  double bfield_{0};
  //Use constant bfield
  bool const_b_field_{true};

  //Transverse Momentum
  double pt_{1.};
  //d0 and z0 are drawn from a gaussian distribution with these resolutions.
  double d0sigma_{1.};
  double z0sigma_{1.};
 
  //Stepping size (in mm)
  double propagator_step_size_{200.};

  //The perigee location used for the initial propagator states generation
  std::vector<double> perigee_location_{0.,0.,0.};

  //The extrapolation surface
  bool use_extrapolate_location_{true};
  std::vector<double> extrapolate_location_{0.,0.,0.};

  //The hit collection to use for track reconstruction
  std::string hit_collection_{"TaggerSimHits"};

  //The interpolated bfield
  std::shared_ptr<InterpolatedMagneticField3> sp_interpolated_bField_;
  std::string bfieldMap_;
  
  //The propagator
  std::shared_ptr<Propagator> propagator_;
  
  //The options
  std::shared_ptr<PropagatorOptions> options_;
  
  //The propagator steps writer
  std::shared_ptr<PropagatorStepWriter> writer_;

  //Outname of the propagator test
  std::string steps_outfile_path_{""};

  //This could be a vector
  //The mapping between layers and Acts::Surface
  std::unordered_map<unsigned int, const Acts::Surface*> layer_surface_map_;

  //Some histograms

  TH1F* histo_p_;
  TH1F* histo_d0_;
  TH1F* histo_z0_;
  TH1F* histo_phi_;
  TH1F* histo_theta_;

  
  TH1F* h_p_;
  TH1F* h_d0_;
  TH1F* h_z0_;
  TH1F* h_phi_;
  TH1F* h_theta_;

  TH1F* h_p_truth_;
  TH1F* h_d0_truth_;
  TH1F* h_z0_truth_;
  TH1F* h_phi_truth_;
  TH1F* h_theta_truth_;

  TH2F* h_tgt_scoring_x_y_;
  TH1F* h_tgt_scoring_z_;


  //Tracker mapping.
  //Each key represent the layer index and each entry is the vector of surfaces that one wants to add to the same layer
  //In this way we can pass multiple surfaces to the same layer to the builder.
  
  std::map<std::string, std::vector<Acts::CuboidVolumeBuilder::SurfaceConfig > > tracker_layout;
  
}; // TrackingGeometryMaker
    

} // namespace sim
} // namespace tracking

#endif // TRACKING_SIM_TRACKINGGEOMAKER_H_

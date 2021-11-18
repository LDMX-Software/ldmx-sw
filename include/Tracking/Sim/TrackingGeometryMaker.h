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

//Utils
#include "Acts/Utilities/Logger.hpp"
#include "Acts/Definitions/Units.hpp"

//dd4hep
#include "Acts/Plugins/DD4hep/ActsExtension.hpp"
#include "Acts/Plugins/DD4hep/DD4hepLayerBuilder.hpp"
#include "Acts/Plugins/DD4hep/DD4hepDetectorElement.hpp"

//geometry
#include "Acts/Geometry/CuboidVolumeBuilder.hpp"
#include "Acts/Geometry/GeometryContext.hpp"

//magfield
#include "Acts/MagneticField/MagneticFieldContext.hpp"

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
  
  std::shared_ptr<PropagatorOptions> TestPropagatorOptions();
  
 private:
  /// The detector
  dd4hep::Detector* detector_{nullptr};
  Acts::GeometryContext gctx_;
  Acts::MagneticFieldContext bctx_;
  
  //If we want to dump the tracking geometry
  int dumpobj_ {0};


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
  
  //The propagator
  std::shared_ptr<Propagator> propagator_;
  
  //The options
  std::shared_ptr<PropagatorOptions> options_;
  
  //The propagator steps writer
  std::shared_ptr<PropagatorStepWriter> writer_;

  //Outname of the propagator test
  std::string steps_outfile_path_{""};
  
}; // TrackingGeometryMaker
    

} // namespace sim
} // namespace tracking

#endif // TRACKING_SIM_TRACKINGGEOMAKER_H_

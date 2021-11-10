#ifndef TRACKING_SIM_TRACKINGGEOMAKER_H_
#define TRACKING_SIM_TRACKINGGEOMAKER_H_

/*~~~~~~~~~~~~~~~*/
/*   Framework   */
/*~~~~~~~~~~~~~~~*/
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"

/*~~~~~~~~~~~~*/
/*   DD4hep   */
/*~~~~~~~~~~~~*/
#include "DD4hep/Detector.h"

/*~~~~~~~~~~*/
/*   ROOT   */
/*~~~~~~~~~~*/
#include "TGeoMatrix.h"

/*~~~~~~~~~~~~*/
/*    ACTS    */
/*~~~~~~~~~~~~*/

#include "Acts/Utilities/Logger.hpp"
#include "Acts/Plugins/DD4hep/ActsExtension.hpp"
#include "Acts/Plugins/DD4hep/DD4hepLayerBuilder.hpp"
#include "Acts/Plugins/DD4hep/DD4hepDetectorElement.hpp"
#include "Acts/Geometry/CuboidVolumeBuilder.hpp"
#include "Acts/Definitions/Units.hpp"
#include "Acts/Geometry/GeometryContext.hpp"
#include "Acts/Geometry/TrackingVolume.hpp"
#include "Acts/Geometry/TrackingGeometryBuilder.hpp"
#include <Acts/Geometry/TrackingGeometry.hpp>


#include "Acts/Material/HomogeneousSurfaceMaterial.hpp"
#include "Acts/Material/HomogeneousVolumeMaterial.hpp"
#include "Acts/Material/Material.hpp"
#include "Acts/Surfaces/RectangleBounds.hpp"


///Visualization
#include <Acts/Visualization/ObjVisualization3D.hpp>
#include <Acts/Visualization/GeometryView3D.hpp>
#include <Acts/Visualization/ViewConfig.hpp>


//This should be changed in the new version
//#include "Acts/Material/MaterialProperties.hpp"
#include "Acts/Material/MaterialSlab.hpp"
#include "Acts/Material/HomogeneousSurfaceMaterial.hpp"

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
   * Run the processor and create a collection of results which
   * indicate if a charge particle can be found by the recoil tracker.
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

private:
  /// The detector
  dd4hep::Detector* detector_{nullptr};
  Acts::GeometryContext m_gctx;
  int dumpobj_ {0};
}; // TrackingGeometryMaker
    

} // namespace sim
} // namespace tracking

#endif // TRACKING_SIM_TRACKINGGEOMAKER_H_

#ifndef TRACKING_RECO_LDMXTRACKINGGEOMETRY_H_
#define TRACKING_RECO_LDMXTRACKINGGEOMETRY_H_


//--- DD4hep ---//
#include "DD4hep/Detector.h"
#include "DD4hep/DetElement.h"

//TODO get rid of this
//--- ROOT ---//
#include "TGeoMatrix.h"

//--- ACTS ---//
//dd4hep
#include "Acts/Plugins/DD4hep/ActsExtension.hpp"
#include "Acts/Plugins/DD4hep/DD4hepLayerBuilder.hpp"      //needed?
#include "Acts/Plugins/DD4hep/DD4hepDetectorElement.hpp"   //needed?


//geometry
#include "Acts/Plugins/TGeo/TGeoPrimitivesHelper.hpp"
#include "Acts/Geometry/CuboidVolumeBuilder.hpp"
#include "Acts/Geometry/GeometryContext.hpp"
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

namespace tracking {
namespace reco {


class LdmxTrackingGeometry {
  
 public:
  
  LdmxTrackingGeometry(dd4hep::Detector* detector,
                       Acts::GeometryContext* gctx);
  
  
  std::shared_ptr<const Acts::TrackingGeometry> getTG(){return tGeometry_;};

  void dumpGeometry(const std::string& outputDir );
  void getSurfaces(std::vector<const Acts::Surface*>& surfaces,
                   std::shared_ptr<const Acts::TrackingGeometry> trackingGeometry);
  
  
 private:

  //The tracking geometry
  std::shared_ptr<const Acts::TrackingGeometry> tGeometry_;
  
  //Use smart pointer instead
  dd4hep::Detector      *detector_{nullptr};
  Acts::GeometryContext *gctx_{nullptr};

  bool dumpobj_{0};
  bool debug_{false};

  //Tracker mapping.
  //Each key represent the layer index and each entry is the vector of surfaces that one wants to add to the same layer
  //In this way we can pass multiple surfaces to the same layer to the builder.
  
  std::map<std::string, std::vector<Acts::CuboidVolumeBuilder::SurfaceConfig > > tracker_layout;

  
  void collectSensors_dd4hep(dd4hep::DetElement& detElement,
                                                   std::vector<dd4hep::DetElement>& sensors);
  void collectSubDetectors_dd4hep(dd4hep::DetElement& detElement,
                                  std::vector<dd4hep::DetElement>& subdetectors);
  void resolveSensitive(
      const dd4hep::DetElement& detElement,
      std::vector<std::shared_ptr<const Acts::Surface>>& surfaces,bool force) const;
  std::shared_ptr<const Acts::Surface>
  createSensitiveSurface(
      const dd4hep::DetElement& detElement) const ;
  Acts::Transform3 convertTransform(
      const TGeoMatrix* tGeoTrans) const;
    
  Acts::CuboidVolumeBuilder::VolumeConfig volumeBuilder_dd4hep(dd4hep::DetElement& subdetector,Acts::Logging::Level logLevel);
    

  
};
}
}





#endif

#ifndef TRACKING_RECO_BASETRACKINGGEOMETRY_H_
#define TRACKING_RECO_BASETRACKINGGEOMETRY_H_



//Acts
#include <Acts/Geometry/TrackingGeometry.hpp>
#include <Acts/Surfaces/DiamondBounds.hpp>
#include <Acts/Surfaces/PlaneSurface.hpp>
#include <Acts/Geometry/CuboidVolumeBuilder.hpp>
#include <Acts/Geometry/TrackingGeometryBuilder.hpp>
#include "Acts/Definitions/Units.hpp"
#include "Acts/Material/HomogeneousSurfaceMaterial.hpp"
#include "Acts/Material/HomogeneousVolumeMaterial.hpp"


//Visualization
#include <Acts/Visualization/ObjVisualization3D.hpp>
#include <Acts/Visualization/GeometryView3D.hpp>
#include <Acts/Visualization/ViewConfig.hpp>


//G4
#include <G4GDMLParser.hh>
#include <G4VPhysicalVolume.hh>
#include <G4LogicalVolume.hh>
#include <G4Types.hh>
#include <G4Box.hh>
#include <G4Polyhedra.hh>
#include <G4Material.hh>

#include <string>
#include <boost/filesystem.hpp>

namespace tracking {
namespace reco {

class BaseTrackingGeometry {
  
 public:
  
  BaseTrackingGeometry(std::string gmdlfile,
                       Acts::GeometryContext* gctx,
                       bool debug = false);
  
  G4VPhysicalVolume* findDaughterByName(G4VPhysicalVolume* pvol, G4String name);
  void getAllDaughters(G4VPhysicalVolume* pvol);
  
  static bool compareZlocation(const G4VPhysicalVolume& pvol_a, const G4VPhysicalVolume& pvol_b) {
    return (pvol_a.GetTranslation().z() < pvol_b.GetTranslation().z());
  };
  
  void ConvertG4Rot(const G4RotationMatrix* g4rot, Acts::RotationMatrix3& rot);
  Acts::Vector3 ConvertG4Pos(const G4ThreeVector& g4pos);
  
  void dumpGeometry(const std::string& outputDir);
  
  std::shared_ptr<const Acts::TrackingGeometry> getTG() {return tGeometry_;};

  Acts::Transform3 GetTransform(const G4VPhysicalVolume& phex,
                                bool toTrackingFrame = false);

  Acts::Transform3 toTracker(const Acts::Transform3& trans);

  //Tagger tracker: vol=2 , layer = [2,4,6,8,10,12,14], sensor=[1,2]
  //Recoil tracker: vol=3 , layer = [2,4,6,8,10,12],    sensor=[1,2,3,4,5,6,7,8,9,10]
  void makeLayerSurfacesMap();
  
  void getSurfaces(std::vector<const Acts::Surface*>& surfaces);

  const Acts::Surface* getSurface(int layerid)  {
    return layer_surface_map_.at(layerid);
  }
  
  std::unordered_map<unsigned int, const Acts::Surface*> layer_surface_map_;
  
 protected:

  //The rotation matrices to go from global to tracking frame.
  Acts::RotationMatrix3 x_rot_, y_rot_;
  Acts::GeometryContext* gctx_;
  bool debug_{false};
  std::shared_ptr<const Acts::TrackingGeometry> tGeometry_{nullptr};
  std::string gdml_{""};
  G4VPhysicalVolume* fWorldPhysVol_{nullptr};
  
};

}
}

#endif

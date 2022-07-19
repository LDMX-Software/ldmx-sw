#ifndef TRACKING_RECO_ECALTRACKINGGEOMETRY_H_
#define TRACKING_RECO_ECALTRACKINGGEOMETRY_H_

//Acts
#include <Acts/Geometry/TrackingGeometry.hpp>
#include <Acts/Surfaces/DiamondBounds.hpp>
#include <Acts/Surfaces/PlaneSurface.hpp>
#include <Acts/Geometry/CuboidVolumeBuilder.hpp>
#include <Acts/Geometry/TrackingGeometryBuilder.hpp>
#include "Acts/Definitions/Units.hpp"

//Material
#include "Acts/Material/Material.hpp"
#include "Acts/Material/MaterialSlab.hpp"
#include "Acts/Material/HomogeneousSurfaceMaterial.hpp"

//Visualization
#include <Acts/Visualization/ObjVisualization3D.hpp>
#include <Acts/Visualization/GeometryView3D.hpp>
#include <Acts/Visualization/ViewConfig.hpp>

//ROOT
#include "TGeoManager.h"
#include "TGeoNode.h"
#include "TGeoBBox.h"
#include "TGeoMatrix.h"
#include "TKey.h"
#include "TString.h"

//G4
#include <G4GDMLParser.hh>
#include <G4VPhysicalVolume.hh>
#include <G4LogicalVolume.hh>
#include <G4Types.hh>
#include <G4Polyhedra.hh>
#include <G4Material.hh>

#include <string>
#include <boost/filesystem.hpp>

namespace tracking {
namespace reco {

class EcalTrackingGeometry {
  
 public:
  EcalTrackingGeometry(std::string gdmlfile,
                       Acts::GeometryContext* gctx, bool debug = false);
  
  std::shared_ptr<const Acts::TrackingGeometry> getTG(){return tGeometry_;};

  void dumpGeometry(const std::string& outputDir );
  
 private:
  TGeoManager* _geo{nullptr};
  bool _debug{false};
  std::shared_ptr<const Acts::TrackingGeometry> tGeometry_;
  
  G4VPhysicalVolume* findDaughterByName(G4VPhysicalVolume* pvol, G4String name);
  void getAllDaughters(G4VPhysicalVolume* pvol);
  static bool compareZlocation(const G4VPhysicalVolume& pvol_a, const G4VPhysicalVolume& pvol_b) {

    return (pvol_a.GetTranslation().z() < pvol_b.GetTranslation().z());
    
  };
  void getComponentRing(std::string super_layer_name,
                        std::string component_type,
                        std::vector<std::reference_wrapper<G4VPhysicalVolume>>& components);
  
  G4VPhysicalVolume* _ECAL;
  std::shared_ptr<const Acts::Surface> convertHexToActsSurface(const G4VPhysicalVolume& phex);
  Acts::GeometryContext* gctx_;

  void ConvertG4Rot(const G4RotationMatrix& g4rot, Acts::RotationMatrix3& rot);
  Acts::Vector3 ConvertG4Pos(const G4ThreeVector& g4pos);
  
  Acts::RotationMatrix3 x_rot_, y_rot_;
  Acts::CuboidVolumeBuilder::LayerConfig buildLayerConfig( const std::vector<std::shared_ptr<const Acts::Surface>>& rings,
                                                           double clearance = 0.001,
                                                           bool active = true); 
  
  
};

class Ring {
  
 public:
  Ring(const std::vector<G4VPhysicalVolume*>& components,
       double z_location,
       std::string material) {
    _components = components;
    _z_location = z_location;
    _material = material;
  }
  
 private:
  std::vector<G4VPhysicalVolume*> _components;
  double _z_location;
  std::string _material;
};


}
}

#endif

#pragma once

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


#include "Framework/ConditionsObject.h"
#include "Framework/Configure/Parameters.h"
#include "Framework/Exception/Exception.h"

#include "Tracking/geo/DetectorElement.h"

namespace tracking::geo {

/**
 * This class is a abstract base class (ABC) doing common tasks
 * that tracking geometries need done. Right now, in LDMX, there
 * are two (or three) distinct tracking geometries: The tagger
 * tracker, the recoil tracker, and the ECal. Sometimes the
 * tagger and recoil are combined into a single tracker geometry
 * but the ECal is always distinct.
 *
 * While this class inherits from ConditionsObject, it should
 * never have a provider. Only the concrete derived classes
 * should have providers.
 */
class TrackingGeometry : public framework::ConditionsObject {
 public:
  /**
   * @param[in] name the name of this geometry condition object
   * @param[in] gctx the geometry context for this geometry
   * @param[in] gdml the path to the detector GDML to load
   * @param[in] debug whether to print extra information or nah
   */
  TrackingGeometry(const std::string& name, const Acts::GeometryContext& gctx, const std::string& gdml, bool debug);

  /// Destructor.
  virtual ~TrackingGeometry() = default;

  G4VPhysicalVolume* findDaughterByName(G4VPhysicalVolume* pvol, G4String name);
  void getAllDaughters(G4VPhysicalVolume* pvol);
  
  static bool compareZlocation(const G4VPhysicalVolume& pvol_a, const G4VPhysicalVolume& pvol_b) {
    return (pvol_a.GetTranslation().z() < pvol_b.GetTranslation().z());
  };
  
  void ConvertG4Rot(const G4RotationMatrix* g4rot, Acts::RotationMatrix3& rot) const;
  Acts::Vector3 ConvertG4Pos(const G4ThreeVector& g4pos) const;
  
  void dumpGeometry(const std::string& outputDir) const;
  
  std::shared_ptr<const Acts::TrackingGeometry> getTG() const {return tGeometry_;};

  Acts::Transform3 GetTransform(const G4VPhysicalVolume& phex,
                                bool toTrackingFrame = false) const;

  Acts::Transform3 toTracker(const Acts::Transform3& trans) const;

  //Tagger tracker: vol=2 , layer = [2,4,6,8,10,12,14], sensor=[1,2]
  //Recoil tracker: vol=3 , layer = [2,4,6,8,10,12],    sensor=[1,2,3,4,5,6,7,8,9,10]
  void makeLayerSurfacesMap();
  
  void getSurfaces(std::vector<const Acts::Surface*>& surfaces) const;

  const Acts::Surface* getSurface(int layerid) const {
    return layer_surface_map_.at(layerid);
  }
  
  
  std::unordered_map<unsigned int, const Acts::Surface*> layer_surface_map_;

  // Global vector holding all the alignable detector elements of the tracking geometry.
  std::vector<std::shared_ptr<DetectorElement>> detElements;
    
 protected:
  const Acts::GeometryContext& gctx_;
  std::string gdml_{""};
  bool debug_{false};
  //The rotation matrices to go from global to tracking frame.
  Acts::RotationMatrix3 x_rot_, y_rot_;
  std::shared_ptr<const Acts::TrackingGeometry> tGeometry_{nullptr};
  G4VPhysicalVolume* fWorldPhysVol_{nullptr};
  
};
}  // namespace tracking::geo

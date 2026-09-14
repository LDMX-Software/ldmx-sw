#pragma once

// Acts
#include <Acts/Geometry/CuboidVolumeBuilder.hpp>
#include <Acts/Geometry/TrackingGeometry.hpp>
#include <Acts/Geometry/TrackingGeometryBuilder.hpp>
#include <Acts/Surfaces/DiamondBounds.hpp>
#include <Acts/Surfaces/PlaneSurface.hpp>
#include <Acts/Surfaces/SurfaceArray.hpp>

#include "Acts/Definitions/Units.hpp"
#include "Acts/Material/HomogeneousSurfaceMaterial.hpp"
#include "Acts/Material/HomogeneousVolumeMaterial.hpp"

// Visualization
#include <Acts/Visualization/GeometryView3D.hpp>
#include <Acts/Visualization/ObjVisualization3D.hpp>
#include <Acts/Visualization/ViewConfig.hpp>

// G4
#include <G4Box.hh>
#include <G4GDMLParser.hh>
#include <G4LogicalVolume.hh>
#include <G4Material.hh>
#include <G4Polyhedra.hh>
#include <G4Types.hh>
#include <G4VPhysicalVolume.hh>
#include <boost/filesystem.hpp>
#include <string>

#include "Framework/ConditionsObject.h"
#include "Framework/Configure/Parameters.h"
#include "Framework/Exception/Exception.h"
#include "Framework/Logger.h"
#include "G4RunManager.hh"
#include "G4UIsession.hh"
#include "G4strstreambuf.hh"
#include "Tracking/geo/DetectorElement.h"
#include "Tracking/geo/GeoUtils.h"
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
   */
  TrackingGeometry(const std::string& name, const Acts::GeometryContext& gctx,
                   const std::string& gdml);

  /// Destructor.
  virtual ~TrackingGeometry() = default;

  G4VPhysicalVolume* findDaughterByName(G4VPhysicalVolume* pvol, G4String name);
  void getAllDaughters(G4VPhysicalVolume* pvol);

  static bool compareZlocation(const G4VPhysicalVolume& pvol_a,
                               const G4VPhysicalVolume& pvol_b) {
    return (pvol_a.GetTranslation().z() < pvol_b.GetTranslation().z());
  };

  void convertG4Rot(const G4RotationMatrix* g4rot,
                    Acts::RotationMatrix3& rot) const;
  Acts::Vector3 convertG4Pos(const G4ThreeVector& g4pos) const;

  void dumpGeometry(const std::string& outputDir,
                    const Acts::GeometryContext& gctx) const;

  std::shared_ptr<const Acts::TrackingGeometry> getTG() const {
    return t_geometry_;
  };

  Acts::Transform3 getTransform(const G4VPhysicalVolume& phex,
                                bool toTrackingFrame = false) const;

  Acts::Transform3 toTracker(const Acts::Transform3& trans) const;

  // Tagger tracker: vol=2 , layer = [2,4,6,8,10,12,14], sensor=[1,2]
  // Recoil tracker: vol=3 , layer = [2,4,6,8,10,12],
  // sensor=[1,2,3,4,5,6,7,8,9,10]
  void makeLayerSurfacesMap();

  void getSurfaces(std::vector<const Acts::Surface*>& surfaces) const;

  const Acts::Surface* getSurface(int layerid) const {
    return layer_surface_map_.at(layerid);
  }

  std::unordered_map<unsigned int, const Acts::Surface*> layer_surface_map_;

  /// Full path to the field map file extracted from the GDML auxiliary data.
  /// Empty if no MagneticField auxiliary block was found.
  const std::string& fieldMapFile() const { return field_map_file_; }

  // Global vector holding all the alignable detector elements of the tracking
  // geometry.
  //  std::vector<std::shared_ptr<DetectorElement>> det_elements_;
  std::vector<std::shared_ptr<tracking::geo::DetectorElement>> det_elements_;

 protected:
  const Acts::GeometryContext& gctx_;
  std::string gdml_{""};
  // The rotation matrices to go from global to tracking frame.
  Acts::RotationMatrix3 x_rot_, y_rot_;
  std::shared_ptr<const Acts::TrackingGeometry> t_geometry_{nullptr};
  G4VPhysicalVolume* f_world_phys_vol_{nullptr};

 private:
  std::string field_map_file_{""};
  enableLogging("TrackingGeometry")
};
}  // namespace tracking::geo

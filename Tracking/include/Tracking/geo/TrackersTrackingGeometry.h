#ifndef TRACKING_RECO_TRACKERSTRACKINGGEOMETRY_H_
#define TRACKING_RECO_TRACKERSTRACKINGGEOMETRY_H_

// Acts

// geometry
#include <Acts/Geometry/TrackingGeometry.hpp>

#include "Acts/Geometry/CuboidVolumeBuilder.hpp"
#include "Acts/Geometry/GeometryContext.hpp"
#include "Acts/Geometry/TrackingGeometryBuilder.hpp"
#include "Acts/Geometry/TrackingVolume.hpp"
#include "Acts/Surfaces/RectangleBounds.hpp"

// Material
#include "Acts/Material/HomogeneousSurfaceMaterial.hpp"
#include "Acts/Material/Material.hpp"
#include "Acts/Material/MaterialSlab.hpp"

/// Visualization
#include <Acts/Visualization/GeometryView3D.hpp>
#include <Acts/Visualization/ObjVisualization3D.hpp>
#include <Acts/Visualization/ViewConfig.hpp>

// G4
#include <G4GDMLParser.hh>
#include <G4LogicalVolume.hh>
#include <G4Material.hh>
#include <G4Polyhedra.hh>
#include <G4Types.hh>
#include <G4VPhysicalVolume.hh>

// Tracking
#include <boost/filesystem.hpp>
#include <string>

#include "Tracking/geo/TrackingGeometry.h"

namespace tracking::geo {

/**
 * forward declaration for friendship
 */
class TrackersTrackingGeometryProvider;

class TrackersTrackingGeometry : public TrackingGeometry {
 public:
  static const std::string NAME;
  void buildTaggerLayoutMap(G4VPhysicalVolume* pvol, std::string surfacename);

  void buildRecoilLayoutMap(G4VPhysicalVolume* pvol, std::string surfacename);

  // Provided a physical volume, extract a silicon rectangular plane surface
  std::shared_ptr<Acts::PlaneSurface> GetSurface(G4VPhysicalVolume* pvol,
                                                 Acts::Transform3 ref_trans);

  Acts::CuboidVolumeBuilder::VolumeConfig buildVolumeConfig(
      const G4VPhysicalVolume* detector,
      const std::map<std::string,
                     std::vector<std::shared_ptr<const Acts::Surface>>>
          layout,
      double tracker_y_length, double tracker_z_length,
      const std::string& volumeName);

  // TODO Implement these
  Acts::CuboidVolumeBuilder::VolumeConfig buildTSVolume() { return {}; }
  Acts::CuboidVolumeBuilder::VolumeConfig buildTargetVolume() { return {}; }

 private:
  friend TrackersTrackingGeometryProvider;
  TrackersTrackingGeometry(const Acts::GeometryContext& gctx,
                           const std::string& gdml, double tracker_y_length,
                           double tracker_z_length);

  G4VPhysicalVolume* tagger_;
  G4VPhysicalVolume* recoil_;

  // I store the layout as a map to distinguish layers/sides
  // They are not too many modules, so it should be ok to use this data
  // structure

  // Tracker mapping.
  // Each key represent the layer index and each entry is the vector of surfaces
  // that one wants to add to the same layer In this way we can pass multiple
  // surfaces to the same layer to the builder.
  std::map<std::string, std::vector<std::shared_ptr<const Acts::Surface>>>
      tagger_layout;
  std::map<std::string, std::vector<std::shared_ptr<const Acts::Surface>>>
      recoil_layout;

 private:
  enableLogging("TrackersTrackingGeometry")
};

}  // namespace tracking::geo

#endif

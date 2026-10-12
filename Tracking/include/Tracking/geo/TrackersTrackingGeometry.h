#ifndef TRACKING_RECO_TRACKERSTRACKINGGEOMETRY_H_
#define TRACKING_RECO_TRACKERSTRACKINGGEOMETRY_H_

// Acts

// geometry
#include <Acts/Geometry/TrackingGeometry.hpp>

#include "Acts/Geometry/CuboidVolumeBuilder.hpp"
#include "Acts/Geometry/GeometryContext.hpp"
#include "Acts/Geometry/TrackingVolume.hpp"

// Material

/// Visualization
#include <Acts/Visualization/GeometryView3D.hpp>

// G4
#include <G4VPhysicalVolume.hh>

// Tracking
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
  std::shared_ptr<Acts::PlaneSurface> getSurfacePtr(G4VPhysicalVolume* pvol,
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

  G4VPhysicalVolume* tagger_{nullptr};
  G4VPhysicalVolume* recoil_{nullptr};

  // I store the layout as a map to distinguish layers/sides
  // They are not too many modules, so it should be ok to use this data
  // structure

  // Tracker mapping.
  // Each key represent the layer index and each entry is the vector of
  // surfaces that one wants to add to the same layer In this way we can pass
  // multiple surfaces to the same layer to the builder.
  std::map<std::string, std::vector<std::shared_ptr<const Acts::Surface>>>
      tagger_layout_;
  std::map<std::string, std::vector<std::shared_ptr<const Acts::Surface>>>
      recoil_layout_;

 private:
  enableLogging("TrackersTrackingGeometry")
};

}  // namespace tracking::geo

#endif

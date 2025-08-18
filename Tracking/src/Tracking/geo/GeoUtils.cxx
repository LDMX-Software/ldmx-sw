#include "Tracking/geo/GeoUtils.h"

namespace tracking::geo {

unsigned int unpackGeometryIdentifier(const Acts::GeometryIdentifier& geoId) {
  unsigned int volume_id = geoId.volume();
  unsigned int layer_id = geoId.layer() / 2;
  unsigned int sensor_id = geoId.sensitive() - 1;
  unsigned int surface_id = volume_id * 1000 + layer_id * 100 + sensor_id;

  return surface_id;
}

Acts::RotationMatrix3 deltaRot(const Acts::Vector3& deltaR) {
  // This is fine because RotationMatrix3 doesn't need to be symmetric
  Acts::RotationMatrix3 rot = Acts::RotationMatrix3::Identity();
  rot(0, 1) = -deltaR(2);
  rot(0, 2) = deltaR(1);
  rot(1, 2) = -deltaR(0);

  rot(1, 0) = -rot(0, 1);
  rot(2, 0) = -rot(0, 2);
  rot(2, 1) = -rot(1, 2);

  return rot;
}

}  // namespace tracking::geo
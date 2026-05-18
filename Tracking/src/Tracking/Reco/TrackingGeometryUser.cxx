#include "Tracking/Reco/TrackingGeometryUser.h"

#include "Acts/MagneticField/ConstantBField.hpp"

namespace tracking::reco {

TrackingGeometryUser::TrackingGeometryUser(const std::string& name,
                                           framework::Process& p)
    : framework::Producer(name, p) {}

const Acts::GeometryContext& TrackingGeometryUser::geometryContext() {
  return getNamedCondition<geo::GeometryContext>().get();
}
const Acts::MagneticFieldContext& TrackingGeometryUser::magneticFieldContext() {
  return getNamedCondition<geo::MagneticFieldContext>().get();
}
const Acts::CalibrationContext& TrackingGeometryUser::calibrationContext() {
  return getNamedCondition<geo::CalibrationContext>().get();
}
const geo::TrackersTrackingGeometry& TrackingGeometryUser::geometry() {
  return getNamedCondition<geo::TrackersTrackingGeometry>();
}

void TrackingGeometryUser::loadBField(const std::vector<double>& map_offset) {
  loadBField(geometry().fieldMapFile(), map_offset);
}

void TrackingGeometryUser::loadBField(const std::string& path,
                                      const std::vector<double>& map_offset) {
  if (path.empty()) {
    ldmx_log(warn) << "No B-field map path provided — using zero B-field";
    b_field_ =
        std::make_shared<Acts::ConstantBField>(Acts::Vector3(0., 0., 0.));
    return;
  }
  auto transform_pos = [map_offset](const Acts::Vector3& pos) {
    return Acts::Vector3(pos(1) + map_offset[0], pos(2) + map_offset[1],
                         pos(0) + DIPOLE_OFFSET + map_offset[2]);
  };
  auto transform_b = [](const Acts::Vector3& field, const Acts::Vector3&) {
    return Acts::Vector3(field(2), field(0), field(1));
  };
  b_field_ = std::make_shared<InterpolatedMagneticField3>(
      loadDefaultBField(path, transform_pos, transform_b));
}

}  // namespace tracking::reco

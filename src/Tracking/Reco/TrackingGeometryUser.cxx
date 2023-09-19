#include "Tracking/Reco/TrackingGeometryUser.h"

namespace tracking::reco {

TrackingGeometryUser::TrackingGeometryUser(const std::string& name, framework::Process& p)
  : framework::Producer(name, p) {}

const Acts::GeometryContext& TrackingGeometryUser::geometry_context() {
  return getNamedCondition<geo::GeometryContext>().get();
}
const Acts::MagneticFieldContext& TrackingGeometryUser::magnetic_field_context() {
  return getNamedCondition<geo::MagneticFieldContext>().get();
}
const Acts::CalibrationContext& TrackingGeometryUser::calibration_context() {
  return getNamedCondition<geo::CalibrationContext>().get();
}
const geo::TrackersTrackingGeometry& TrackingGeometryUser::geometry() {
  return getNamedCondition<geo::TrackersTrackingGeometry>();
}

}

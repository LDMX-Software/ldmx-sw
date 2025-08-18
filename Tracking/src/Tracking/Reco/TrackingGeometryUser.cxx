#include "Tracking/Reco/TrackingGeometryUser.h"

namespace tracking::reco {

TrackingGeometryUser::TrackingGeometryUser(const std::string& name,
                                           framework::Process& p)
    : framework::Producer(name, p) {}

const Acts::GeometryContext& TrackingGeometryUser::geometryContext() {
  return getNamedCondition<geo::GeometryContext>().get();
}
const Acts::MagneticFieldContext&
TrackingGeometryUser::magneticFieldContext() {
  return getNamedCondition<geo::MagneticFieldContext>().get();
}
const Acts::CalibrationContext& TrackingGeometryUser::calibrationContext() {
  return getNamedCondition<geo::CalibrationContext>().get();
}
const geo::TrackersTrackingGeometry& TrackingGeometryUser::geometry() {
  return getNamedCondition<geo::TrackersTrackingGeometry>();
}

}  // namespace tracking::reco

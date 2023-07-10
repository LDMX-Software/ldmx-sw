
#include "Tracking/geo/TrackingGeometry.h"

namespace tracking::geo {

TrackingGeometry::TrackingGeometry(
    const framework::config::Parameters& parameters)
    : framework::ConditionsObject(TrackingGeometry::CONDITIONS_OBJECT_NAME) {
  detector_ = parameters.getParameter<std::string>("detector");
}

}  // namespace tracking::geo

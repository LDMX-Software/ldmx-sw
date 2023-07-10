
#include "Tracking/geo/TrackingGeometryProvider.h"

#include "Framework/ConditionsObject.h"
#include "Framework/ConditionsObjectProvider.h"
#include "Framework/Configure/Parameters.h"
#include "Framework/Process.h"

namespace tracking::geo {

std::pair<const framework::ConditionsObject*, framework::ConditionsIOV>
TrackingGeometryProvider::getCondition(const ldmx::EventHeader& context) {
  return std::make_pair(
      nullptr,
      framework::ConditionsIOV(context.getRun(), context.getRun(), true, true));
}
}  // namespace tracking::geo

DECLARE_CONDITIONS_PROVIDER_NS(tracking::geo, TrackingGeometryProvider);

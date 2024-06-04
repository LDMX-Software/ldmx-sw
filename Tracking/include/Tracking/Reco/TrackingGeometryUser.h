#pragma once

#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"
#include "Tracking/geo/CalibrationContext.h"
#include "Tracking/geo/GeometryContext.h"
#include "Tracking/geo/MagneticFieldContext.h"
#include "Tracking/geo/TrackersTrackingGeometry.h"

namespace tracking::reco {
/**
 * a helper base class providing some methods to shorten
 * access to common conditions used within the tracking
 * reconstruction
 */
class TrackingGeometryUser : public framework::Producer {
 public:
  TrackingGeometryUser(const std::string& name, framework::Process& p);

 protected:
  const Acts::GeometryContext& geometry_context();
  const Acts::MagneticFieldContext& magnetic_field_context();
  const Acts::CalibrationContext& calibration_context();
  const geo::TrackersTrackingGeometry& geometry();

 private:
  /**
   * Templated condition access code for our conditions with static names.
   *
   * We assume that the condition has a constant name stored in
   * ConditionType::NAME
   *
   * @tparam ConditionType type of condition we are retrieving
   * @return condition object
   */
  template <typename ConditionType>
  const ConditionType& getNamedCondition() {
    return getCondition<ConditionType>(ConditionType::NAME);
  }
};

}  // namespace tracking::reco

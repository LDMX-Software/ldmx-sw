#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Acts/MagneticField/MagneticFieldProvider.hpp"
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"
#include "Tracking/Sim/BFieldXYZUtils.h"
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
  const Acts::GeometryContext& geometryContext();
  const Acts::MagneticFieldContext& magneticFieldContext();
  const Acts::CalibrationContext& calibrationContext();
  const geo::TrackersTrackingGeometry& geometry();

  /**
   * Load the interpolated B-field map from @p path and cache it.
   *
   * Uses the standard LDMX→ACTS coordinate transform plus DIPOLE_OFFSET.
   * An optional per-axis offset (in field-map coordinates) can be supplied
   * for systematic studies (equivalent to CKFProcessor's map_offset_).
   *
   * @param path       Path to the field map text file.
   * @param map_offset Optional {dx, dy, dz} offset in field-map coordinates.
   */
  void loadBField(const std::string& path,
                  const std::vector<double>& map_offset = {0., 0., 0.});

  /** Load B-field from the path recorded in the detector GDML. */
  void loadBField(const std::vector<double>& map_offset = {0., 0., 0.});

  /** Return the loaded B-field provider. Null until loadBField() is called. */
  std::shared_ptr<Acts::MagneticFieldProvider> bField() const {
    return b_field_;
  }

 private:
  std::shared_ptr<Acts::MagneticFieldProvider> b_field_{nullptr};

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

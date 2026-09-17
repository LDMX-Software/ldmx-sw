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
   * An optional distortion can be supplied to deliberately mis-place the
   * reconstruction field for systematic studies; the default leaves the field
   * bit-for-bit nominal.
   *
   * @param path       Path to the field map text file.
   * @param distortion Optional mis-placement of the reconstruction field.
   */
  void loadBField(const std::string& path,
                  const BFieldDistortion& distortion = {});

  /** Load B-field from the path recorded in the detector GDML. */
  void loadBField(const BFieldDistortion& distortion = {});

  /**
   * Build a BFieldDistortion from processor configuration.
   *
   * Config is in the LDMX global frame (x bend, y vertical, z beam); this
   * permutes it into the ACTS frame the lookup works in. Parameters:
   *
   * - `bfield_translation` {dx, dy, dz} [mm], default {0, 0, 0}
   * - `bfield_rotation`    {ax, ay, az} [rad], default {0, 0, 0}
   * - `bfield_pivot`       {x, y, z} [mm], default {0, 0, -400}, the map origin
   * - `bfield_scale`       field scaling, default 1
   *
   * A positive translation moves the magnet, so the field at a fixed point
   * becomes the nominal field from further upstream.
   */
  static BFieldDistortion bFieldDistortion(
      const framework::config::Parameters& parameters);

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

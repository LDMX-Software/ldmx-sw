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

void TrackingGeometryUser::loadBField(const BFieldDistortion& distortion) {
  loadBField(geometry().fieldMapFile(), distortion);
}

void TrackingGeometryUser::loadBField(const std::string& path,
                                      const BFieldDistortion& distortion) {
  if (path.empty()) {
    ldmx_log(warn) << "No B-field map path provided — using zero B-field";
    b_field_ =
        std::make_shared<Acts::ConstantBField>(Acts::Vector3(0., 0., 0.));
    return;
  }

  if (distortion.isNominal()) {
    b_field_ = std::make_shared<InterpolatedMagneticField3>(
        loadDefaultBField(path, defaultTransformPos, defaultTransformBField));
    return;
  }

  const Acts::RotationMatrix3 rot{distortion.rotationMatrix()};
  const Acts::RotationMatrix3 rot_inv{rot.transpose()};
  const Acts::Vector3 translation{distortion.translation};
  const Acts::Vector3 pivot{distortion.pivot};
  const double scale{distortion.scale};

  // undo the distortion, then the nominal ACTS -> map transform
  auto transform_pos = [rot_inv, translation, pivot](const Acts::Vector3& pos) {
    return defaultTransformPos(rot_inv * (pos - translation - pivot) + pivot);
  };
  // nominal map -> ACTS transform, then redo the distortion
  auto transform_b = [rot, scale](const Acts::Vector3& field,
                                  const Acts::Vector3& pos) {
    return Acts::Vector3(scale * (rot * defaultTransformBField(field, pos)));
  };

  ldmx_log(info) << "B-field distortion active (LDMX frame): translation ("
                 << translation(1) << ", " << translation(2) << ", "
                 << translation(0) << ") mm, rotation ("
                 << distortion.rotation(1) << ", " << distortion.rotation(2)
                 << ", " << distortion.rotation(0) << ") rad about ("
                 << pivot(1) << ", " << pivot(2) << ", " << pivot(0)
                 << ") mm, scale " << scale;

  b_field_ = std::make_shared<InterpolatedMagneticField3>(
      loadDefaultBField(path, transform_pos, transform_b));
}

BFieldDistortion TrackingGeometryUser::bFieldDistortion(
    const framework::config::Parameters& parameters) {
  const auto translation{
      parameters.get<std::vector<double>>("bfield_translation", {0., 0., 0.})};
  const auto rotation{
      parameters.get<std::vector<double>>("bfield_rotation", {0., 0., 0.})};
  const auto pivot{parameters.get<std::vector<double>>(
      "bfield_pivot", {0., 0., -DIPOLE_OFFSET})};

  auto require_three = [](const std::string& name,
                          const std::vector<double>& v) {
    if (v.size() != 3)
      EXCEPTION_RAISE("BadConf", name + " must have three entries");
  };
  require_three("bfield_translation", translation);
  require_three("bfield_rotation", rotation);
  require_three("bfield_pivot", pivot);

  // LDMX (x, y, z) -> ACTS (z, x, y); a cyclic permutation, so the rotation
  // angles reorder the same way as the positions
  BFieldDistortion distortion;
  distortion.translation =
      Acts::Vector3(translation[2], translation[0], translation[1]);
  distortion.rotation = Acts::Vector3(rotation[2], rotation[0], rotation[1]);
  distortion.pivot = Acts::Vector3(pivot[2], pivot[0], pivot[1]);
  distortion.scale = parameters.get<double>("bfield_scale", 1.);
  return distortion;
}

}  // namespace tracking::reco

#include "Tracking/geo/DetectorElement.h"

#include "Tracking/geo/GeometryContext.h"

namespace tracking::geo {

DetectorElement::~DetectorElement(){};

const Acts::Transform3& DetectorElement::transform(
    const Acts::GeometryContext& gctx) const {
  if (!m_surface_)
    throw std::logic_error("DetectorElement:: Sensor/Element ID not set");

  // The elementId will be valid only after tracking geometry is built
  // I will use this fact to return the default transform in order to build
  // always the same default tracking geometry and modify later the sensor
  // transformations.

  unsigned int element_id = unpackGeometryIdentifier(m_surface_->geometryId());

  // Check if the elementId is valid
  if (element_id > 9999) {
    // elementId not valid: return default transformation
    return m_transform_;
  }

  auto ctx = gctx.get<GeometryContext*>();

  // Found the aligned transform for this sensor
  if ((ctx->alignment_map_).count(element_id) > 0) {
    const Acts::Transform3& c_transform = ctx->alignment_map_[element_id];

    if (false) {
      // TODO : Have this in the logging system
      std::cout << "Aligned transform" << std::endl;
      std::cout << c_transform.translation() << std::endl;
      std::cout << c_transform.rotation() << std::endl;
      std::cout << "Original transform" << std::endl;
      std::cout << m_transform_.translation() << std::endl;
      std::cout << m_transform_.rotation() << std::endl;
    }

    return c_transform;
  }

  else
    return m_transform_;
}

const Acts::Surface& DetectorElement::surface() const {
  if (!m_surface_)
    throw std::logic_error(
        "DetectorElement::Attempted to return reference of null ptr");
  return *m_surface_;
}
Acts::Surface& DetectorElement::surface() {
  if (!m_surface_)
    throw std::logic_error(
        "DetectorElement::Attempted to return reference of null ptr");
  return *m_surface_;
}

// The thickness of the detector element is taken from the center of the
// associated surface
double DetectorElement::thickness() const {
  // return m_thickness;
  auto material = static_cast<const Acts::HomogeneousSurfaceMaterial*>(
      m_surface_->surfaceMaterial());
  return material->materialSlab(Acts::Vector2{0., 0.}).thickness();
}
}  // namespace tracking::geo

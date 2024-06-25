#include "Tracking/geo/DetectorElement.h"

#include "Tracking/geo/GeometryContext.h"

namespace tracking::geo {

DetectorElement::~DetectorElement(){};

const Acts::Transform3& DetectorElement::transform(
    const Acts::GeometryContext& gctx) const {
  if (!m_surface)
    throw std::logic_error("DetectorElement:: Sensor/Element ID not set");

  // The elementId will be valid only after tracking geometry is built
  // I will use this fact to return the default transform in order to build
  // always the same default tracking geometry and modify later the sensor
  // transformations.

  unsigned int elementId = unpackGeometryIdentifier(m_surface->geometryId());

  // Check if the elementId is valid
  if (elementId > 9999) {
    // elementId not valid: return default transformation
    return m_transform;
  }

  auto ctx = gctx.get<GeometryContext*>();

  // Found the aligned transform for this sensor
  if ((ctx->alignment_map).count(elementId) > 0) {
    const Acts::Transform3& c_transform = ctx->alignment_map[elementId];

    if (m_debug) {
      std::cout << "Aligned transform" << std::endl;
      std::cout << c_transform.translation() << std::endl;
      std::cout << c_transform.rotation() << std::endl;
      std::cout << "Original transform" << std::endl;
      std::cout << m_transform.translation() << std::endl;
      std::cout << m_transform.rotation() << std::endl;
    }

    return c_transform;
  }

  else
    return m_transform;
}

const Acts::Surface& DetectorElement::surface() const {
  if (!m_surface)
    throw std::logic_error(
        "DetectorElement::Attempted to return reference of null ptr");
  return *m_surface;
}
Acts::Surface& DetectorElement::surface() {
  if (!m_surface)
    throw std::logic_error(
        "DetectorElement::Attempted to return reference of null ptr");
  return *m_surface;
}

// The thickness of the detector element is taken from the center of the
// associated surface
double DetectorElement::thickness() const {
  // return m_thickness;
  auto material = static_cast<const Acts::HomogeneousSurfaceMaterial*>(
      m_surface->surfaceMaterial());
  return material->materialSlab(Acts::Vector2{0., 0.}).thickness();
}
}  // namespace tracking::geo

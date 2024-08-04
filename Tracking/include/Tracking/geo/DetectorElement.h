#pragma once

#include <iostream>
#include <stdexcept>  // Include for std::logic_error

#include "Acts/Definitions/Algebra.hpp"
#include "Acts/Geometry/DetectorElementBase.hpp"
#include "Acts/Geometry/GeometryContext.hpp"
#include "Acts/Geometry/GeometryIdentifier.hpp"
#include "Acts/Material/HomogeneousSurfaceMaterial.hpp"
#include "Acts/Surfaces/Surface.hpp"
#include "Tracking/geo/GeoUtils.h"

// This class is necessary in order to apply any transformation change passed
// via the geometry context to the final sensitive element.

namespace tracking::geo {

class DetectorElement : public Acts::DetectorElementBase {
 public:
  // The detector element is initialized with the surface initial transformation
  // created from the TrackingGeometry constructor/parser
  // The DetectorElement ACTS mechanism is such that subsequent calls to the
  // transform(GeometryContext) method will return the full transformation valid
  // for a certain IoV via the ConditionsProvider mechanism

  DetectorElement(const std::shared_ptr<Acts::Surface>& surface,
                  const Acts::Transform3& default_transform, double thickness) {
    m_surface = surface;
    m_thickness = thickness;

    // This is the local to global transformation
    m_transform = default_transform;
  }

  ~DetectorElement() override;

  // This method will always return a copy to the uncorrected transformation

  Acts::Transform3 uncorrectedTransform() const { return m_transform; }

  // This method will load the transformation from the geometry context if found
  // otherwise will return the default transform
  // This is very *hot* code, do not place computations in this function in
  // order to keep performance under control.

  // CAUTION:: The tracking geometry + geometry context machinery
  // assumes large enough envelopes / gaps between sensors
  // to allow for not breaking the layers overlaps.
  // In case the corrections to the stored transformations will be too large
  // this assumption will be broken.

  // TODO Current implementation implies always checking the stored map of
  // corrections Could be interesting to cache the transformations and re-update
  // all of them when IoV changes

  const Acts::Transform3& transform(
      const Acts::GeometryContext& gctx) const override;

  const Acts::Surface& surface() const override;

  Acts::Surface& surface() override;

  // The thickness of the detector element is taken from the center of the
  // associated surface
  double thickness() const override;

  Acts::GeometryIdentifier geometryId() const {
    if (!m_surface)
      throw std::logic_error("DetectorElement:: surface not assigned");

    return (m_surface->geometryId());
  }

 private:
  // cache
  Acts::Transform3 m_transform = Acts::Transform3::Identity();

  std::shared_ptr<Acts::Surface> m_surface;
  double m_thickness;
  bool m_debug{false};
};

}  // namespace tracking::geo

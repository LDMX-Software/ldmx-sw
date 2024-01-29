#pragma once

#include "Acts/Geometry/DetectorElementBase.hpp"
#include "Acts/Geometry/GeometryIdentifier.hpp"
#include "Acts/Definitions/Algebra.hpp"
#include "Acts/Geometry/GeometryContext.hpp"
#include "Tracking/geo/GeometryContext.h"
#include "Tracking/geo/GeoUtils.h"
#include <stdexcept> // Include for std::logic_error
#include <iostream>

// This class is necessary in order to apply any transformation change passed via the geometry context to the final sensitive element.

namespace tracking::geo {

class DetectorElement : public Acts::DetectorElementBase {
  
 public:

  // The detector element is initialized with the surface initial transformation
  // created from the TrackingGeometry constructor/parser
  // The DetectorElement ACTS mechanism is such that subsequent calls to the
  // transform(GeometryContext) method will return the full transformation valid
  // for a certain IoV via the ConditionsProvider mechanism
  
  DetectorElement(const std::shared_ptr<Acts::Surface>& surface,
                  const Acts::Transform3& default_transform,
                  double thickness) {
    m_surface   = surface;
    m_thickness = thickness;
    m_transform = default_transform;
  }
  
  // This method will always return the uncorrected transformation
  
  const Acts::Transform3& uncorrectedTransform(const Acts::GeometryContext& gctx) const {
    return m_transform;
  }
  
  // This method will load the transformation from the geometry context if found
  // otherwise will return the default transform

  // TODO Current implementation implies always checking the stored map of corrections
  // Could be interesting to cache the transformations and re-update all of them when IoV changes
  
  const Acts::Transform3& transform(const Acts::GeometryContext& gctx) const override {
    
    if (!m_surface)
      throw std::logic_error("DetectorElement:: Sensor/Element ID not set");


    unsigned int elementId = unpackGeometryIdentifier(m_surface->geometryId());
    
    std::cout<<"casting to GeometryContext* for sensor "<< elementId<<std::endl;
    
    auto ctx = gctx.get<GeometryContext*>();
    
    if ((ctx->alignment_map).count(elementId) > 0) {
      
      std::cout<<"Retrieving corrections for "<<elementId<<std::endl;
      
      Acts::Transform3 correction = ctx->alignment_map.at(elementId);
      std::cout<<"Successfully obtained corrections for "<<elementId<<std::endl;
      
      std::cout<<"Local translation correction"<<std::endl;
      std::cout<<correction.translation()<<std::endl;
      std::cout<<"Local rotation correction"<<std::endl;
      std::cout<<correction.rotation()<<std::endl;
      
      return m_transform;
    }
    else
      return m_transform;
    
  }
  
  const Acts::Surface& surface() const override {
    if (!m_surface)
      throw std::logic_error("DetectorElement::Attempted to return reference of null ptr");
    return *m_surface;
  };
  Acts::Surface& surface() override {
    if (!m_surface)
      throw std::logic_error("DetectorElement::Attempted to return reference of null ptr");
    return *m_surface;
  };
  
  // The thickness of the detector element is taken from the center of the associated surface
  double thickness() const override {
    //return m_thickness;
    return m_surface->surfaceMaterial()->materialSlab(Acts::Vector2{0.,0.}).thickness();
  };
  
  
  Acts::GeometryIdentifier geometryId() const {
    if (!m_surface)
      throw std::logic_error("DetectorElement:: surface not assigned");
    
    return(m_surface->geometryId());
    
  }
  
 private:
  
  Acts::Transform3 m_transform = Acts::Transform3::Identity();
  std::shared_ptr<Acts::Surface> m_surface;
  double m_thickness;
};

} //namespace

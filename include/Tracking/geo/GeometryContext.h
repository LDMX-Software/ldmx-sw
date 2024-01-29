#pragma once

#include <Acts/Geometry/TrackingGeometry.hpp>
#include "Acts/Definitions/Algebra.hpp"
#include "Tracking/geo/GeoUtils.h"
#include "Framework/ConditionsObject.h"

namespace tracking::geo {

/// class name of provider
class GeometryContextProvider;

/**
 * The context for a specific geometry
 *
 * The context is both the object used as a condition
 * and the provider of that condition. We could separate
 * the two if a new context needs to be constructed when
 * the conditions are updated but for now keeping them
 * together reduces the amount of code needed.
 */
class GeometryContext : public framework::ConditionsObject {
 public:


  /** TODO it should be private == KEEPING IT PUBLIC FOR TESTING PURPOSE*/
  /**
   * This constructor is where parameters would be passed into the
   * Geometry context from the provider. Right now, there aren't
   * any parameters to pass and so it is just default constructed.
   * We still have it be private so that only the provider can
   * make a new one.
   */
  

  GeometryContext();

  //Some testing functionality
  
  //deltaT = (tu, tv, tw)
  //deltaR = (ru, rv, rw)
  
  //         /  1    -rw   rv  \
  //deltaR = |  rw    1   -ru  |
  //         \ -rv    ru   1   /
  
  void addAlignCorrection(unsigned int sensorId,
                          const Acts::Vector3 deltaT,
                          const Acts::Vector3 deltaR) {
    
    Acts::Translation3 deltaTranslation{deltaT};
    Acts::RotationMatrix3 rot = deltaRot(deltaR);
    
    Acts::Transform3 correction(deltaTranslation * rot);
    
    alignment_map[sensorId] = correction;
  }
  
  std::unordered_map<unsigned int, Acts::Transform3> alignment_map;
  
  
  /// Conditions object name
  static const std::string NAME;
  /**
   * get an Acts::GeometryContext wrapping the pointer to the instance
   * of this GeometryContext
   *
   * @note For future developers, the conditions object should handle
   * the validity of the geometry context. In other words, if someone
   * has a valid handle to a GeometryContext object, they should be able to
   * call this function without worry.
   */
  const Acts::GeometryContext& get() const;
 private:
  /// the provider is a friend and so it can make one
  friend class GeometryContextProvider;

  /**
   * Wrap this instance in an Acts::GeometryContext any object for passing
   * it down to the various acts tools
   */
  Acts::GeometryContext acts_gc_;

};

}

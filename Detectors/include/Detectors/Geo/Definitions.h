#ifndef DETECTORS_GEO_DEFINITIONS_H
#define DETECTORS_GEO_DEFINITIONS_H

//---< Eigen >---//
#include <Eigen/Dense>
#include <Eigen/Geometry>

/// Format for priting out Eigen vectors
static Eigen::IOFormat simple(Eigen::FullPrecision, 0, ", ", ", ", "", "", "[ ", " ]"); 

/// typedef for a 3D eigen vector
typedef Eigen::Matrix<double, 3, 1> Vector3D;

/// typedef for a 3D translation
namespace eigen {
  typedef Eigen::Transform<double, 3, Eigen::Affine> Transform3D; 
}

namespace detectors {
namespace geo {

/// Conversion from cm to mm i.e. from DD4hep to Tracking
inline static float mm{10};

} // namespace geo
} // namespace detectors

#endif // DETECTORS_GEO_DEFINITIONS_H

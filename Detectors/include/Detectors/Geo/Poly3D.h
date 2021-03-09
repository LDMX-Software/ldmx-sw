#ifndef DETECTORS_GEO_POLY3D_H
#define DETECTORS_GEO_POLY3D_H

//---< C++ >---//
#include <vector>

//---< Detectors >---//
#include "Detectors/Geo/Definitions.h"

namespace detectors {
namespace geo {

/**
 * Class representing a polygon. 
 */
class Poly3D {
public:
  /**
   * Constructor. Takes a set of vertices (Vector3D objects) and constructs a
   *  polygon. This requires at least 4 vertices to be specified.
   *
   * @param points C++ vector of vertices
   */
  Poly3D(std::vector<Vector3D> vertices);

  /// Default destructor
  ~Poly3D() = default;

  /// @return a vector of Vector3D objects with the vertices of this polygon
  std::vector<Vector3D> vertices();

  /// Update the normal such that the polygon is facing away from the origin.
  void faceOutward();

private:
  /// Vector containing the vertices
  std::vector<Vector3D> vertices_;

  /// Normal to the polygon
  Vector3D normal_;

  /// Distance from the origin
  double distance_{0};
};

} // namespace geo
} // namespace detectors

#endif // DETECTORS_GEO_POLY3D_H

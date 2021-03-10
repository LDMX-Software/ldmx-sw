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
  std::vector<Vector3D> vertices() const { return vertices_; };

  /// Update the normal such that the polygon is facing away from the origin.
  void faceOutward();

  /// @return The normal to this polygon
  Vector3D normal() const { return normal_; }

  /// @return The distance from the origin
  double distance() const { return distance_; }

  friend std::ostream &operator<<(std::ostream &output, const Poly3D &poly) {
    output << "[ detectors::geo::Poly3D ]:\n Normal = "
           << poly.normal().format(simple) << std::endl
           << " Distance = " << poly.distance() << " mm " << std::endl
           << " Vertices: " << std::endl;
    for (auto &vertex : poly.vertices())
      output << "\t" << vertex.format(simple) << " mm " << std::endl;

    return output;
  }

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

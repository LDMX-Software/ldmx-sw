#include "Detectors/Geo/Poly3D.h"

namespace detectors {
namespace geo {

Poly3D::Poly3D(std::vector<Vector3D> vertices) {

  // Require at least 4 vertices. If less are passes, throw an exception.
  if (vertices.size() < 3) {
    // TODO: throw an exception
  }

  // TODO: Check that the points are coplanar

  vertices_ = vertices;

  // Use the vertices to find the normal and distance from the origin to the
  // poly.
  auto v1{vertices[1] - vertices[0]};
  auto v2{vertices[2] - vertices[1]};
  normal_ = v1.cross(v2).normalized();
  distance_ = normal_.dot(vertices[0]);
  // std::cout << "[ Poly3D ]: Normal: " << normal_ << std::endl;
  // std::cout << "[ Poly3D ]: Distance: " << distance_ << std::endl;
}

void Poly3D::faceOutward() {
  if (distance_ < 0) {
    normal_ *= -1;
    distance_ *= -1;
    //std::cout << "[ Poly3D::faceOutward ]: Normal: " << normal_ << std::endl;
    //std::cout << "[ Poly3D::faceOutward ]: Distance: " << distance_
    //          << std::endl;
  }
}

} // namespace geo
} // namespace detectors

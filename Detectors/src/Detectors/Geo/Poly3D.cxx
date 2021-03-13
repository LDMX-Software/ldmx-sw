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
}

void Poly3D::faceOutward() {
  if (distance_ < 0) {
    normal_ *= -1;
    distance_ *= -1;
  }
}

Poly3D Poly3D::transformed(eigen::Transform3D transform) {

  std::vector<Vector3D> transformed_points;
  for (auto &vertex : vertices_) {
    transformed_points.push_back(transform * vertex);
  }

  // TODO: Check the normal

  return Poly3D(transformed_points);
}

} // namespace geo
} // namespace detectors

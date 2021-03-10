#include "Detectors/Geo/Polyhedron.h"

namespace detectors {
namespace geo {

std::vector<Poly3D> Polyhedron::getFacesNormalTo(Vector3D normal) {
  
  // Container for the faces normal to the vector
  std::vector<Poly3D> faces;
  
  // Loop through all of the faces and check if they are normal to the given
  // vector.  If so, add them to the list. 
  for (auto& face : getFaces()) {
    if (normal.cross(face.normal()).norm() < ANGULAR_TOLERANCE) { 
      if (normal.dot(face.normal()) > 0) {
        faces.push_back(face);
      }
    } 
  }

  return faces; 
}

} // namespace geo
} // namespace detectors

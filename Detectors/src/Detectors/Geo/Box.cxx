#include "Detectors/Geo/Box.h"

//---< Detectors >---//
#include "Detectors/Geo/Definitions.h" 

namespace detectors {
namespace geo {

Box::Box(const std::string &name, double x_half, double y_half, double z_half) {
  m_element = dd4hep::Box(name, x_half, y_half, z_half).ptr();
}

std::vector<Poly3D> Box::getFaces() {

  // Container for all faces of this box 
  std::vector<Poly3D> faces; 

  // Find all of the vertices of this box and construct the faces as polygons.  
  for (int ix{-1}; ix <= 1; ix += 2) {
    std::vector<Vector3D> vertices;
    vertices.push_back(Vector3D(ix * halfX(), halfY(), halfZ()));
    vertices.push_back(Vector3D(ix * halfX(), -halfY(), halfZ()));
    vertices.push_back(Vector3D(ix * halfX(), -halfY(), -halfZ()));
    vertices.push_back(Vector3D(ix * halfX(), halfY(), -halfZ()));
    faces.push_back(Poly3D(vertices));
  }

  for (int iy{-1}; iy <= +1; iy += 2) {
    std::vector<Vector3D> vertices;
    vertices.push_back(Vector3D(+halfX(), iy * halfY(), +halfZ()));
    vertices.push_back(Vector3D(+halfX(), iy * halfY(), -halfZ()));
    vertices.push_back(Vector3D(-halfX(), iy * halfY(), -halfZ()));
    vertices.push_back(Vector3D(-halfX(), iy * halfY(), +halfZ()));
    faces.push_back(Poly3D(vertices)); 
  }

  for (int iz{-1}; iz <= +1; iz += 2) {
    std::vector<Vector3D> vertices;
    vertices.push_back(Vector3D(+halfX(), +halfY(), iz * halfZ()));
    vertices.push_back(Vector3D(-halfX(), +halfY(), iz * halfZ()));
    vertices.push_back(Vector3D(-halfX(), -halfY(), iz * halfZ()));
    vertices.push_back(Vector3D(+halfX(), -halfY(), iz * halfZ()));
    faces.push_back(Poly3D(vertices)); 
  }

  // Loop through all of the faces and make sure the normals are pointing 
  // away from the origin i.e. the faces are facing outward
  for (auto& face : faces) face.faceOutward(); 

  return faces; 
}

} // namespace geo
} // namespace detectors

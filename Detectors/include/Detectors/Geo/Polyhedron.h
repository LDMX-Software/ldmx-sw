#ifndef DETECTORS_GEO_POLYHEDRON_H
#define DETECTORS_GEO_POLYHEDRON_H

//---< C++ >---//
#include <vector>

//---< Detectors >---//
#include "Detectors/Geo/Definitions.h"
#include "Detectors/Geo/Poly3D.h"

namespace detectors {
namespace geo {

/**
 * Abstract class representing a bounded convex 3D polyhedron.
 *
 * This class should not be instantiated directly.
 */
class Polyhedron {

public:
  /// Default constructor
  Polyhedron() = default;

  /// Default destructor
  ~Polyhedron() = default;

  /**
   * Retrieve the faces of this polyhedron as a list of polygons.  This is a
   *  pure virtual method and should be implemented by derived classes.
   *
   * @return Vector containing the faces of this polyhedron as polygons.
   */
  virtual std::vector<Poly3D> getFaces() = 0;

  /**
   * Retrieve the faces that are normal to the given vector.
   *
   * @param normal Normal vector
   *
   * @return Vector containing the faces of this polyhedron that are normal to
   *  the given vector.
   */
  std::vector<Poly3D> getFacesNormalTo(const Vector3D normal);

 private: 

  /// Angular tolerance 
  static constexpr double ANGULAR_TOLERANCE{1e-9}; 
};
} // namespace geo
} // namespace detectors

#endif // DETECTORS_GEO_POLYHEDRON_H

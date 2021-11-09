#ifndef DETECTORS_GEO_BOX_H
#define DETECTORS_GEO_BOX_H

//---< DD4hep >---//
#include "DD4hep/Shapes.h"

//---< Detectors >---//
#include "Detectors/Geo/Definitions.h"
#include "Detectors/Geo/Poly3D.h"
#include "Detectors/Geo/Polyhedron.h"

namespace detectors {
namespace geo {

class Box : public dd4hep::Solid_type<TGeoBBox>, public Polyhedron {

public:
  /// Constructor to be used with an existing object
  template <typename Q> Box(const Q *p) : Solid_type<TGeoBBox>(p) {}
  /// Copy Constructor to be used with an existing object handle
  template <typename Q> Box(const Handle<Q> &e) : Solid_type<TGeoBBox>(e) {}

  /**
   */
  Box(const std::string &name, double x_half, double y_half, double z_half);

  /// Default destructor
  ~Box() = default;

  ///
  double halfX() const { return access()->GetDX() * detectors::geo::mm; }

  ///
  double halfY() const { return access()->GetDY() * detectors::geo::mm; }

  ///
  double halfZ() const { return access()->GetDZ() * detectors::geo::mm; }

  /**
   */
  std::vector<Poly3D> getFaces();

  friend std::ostream &operator<<(std::ostream &output, const Box &box) {
    output << "[ detectors::geo::Box ]: Name: " << box.name()
           << " x-half: " << box.halfX() << " y-half: " << box.halfY()
           << " z-half: " << box.halfZ();
    return output;
  }
};
} // namespace geo
} // namespace detectors

#endif // DETECTORS_GEO_BOX_H

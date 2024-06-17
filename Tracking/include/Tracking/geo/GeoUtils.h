#pragma once

#include "Acts/Definitions/Algebra.hpp"
#include "Acts/Geometry/GeometryIdentifier.hpp"

namespace tracking::geo {

/**
 * The geometry identifier will return vol=0 and lay=0 when it is not valid.
 * The geometry identifier is only valid after building the tracking geometry.
 */

unsigned int unpackGeometryIdentifier(const Acts::GeometryIdentifier& geoId);

// I use the same ATLAS convention (opposite to MPII)
/*
deltaR = (ru, rv, rw)
          /  1    -rw   rv  \
deltaR => |  rw    1   -ru  |
          \ -rv    ru   1   /
 */

Acts::RotationMatrix3 deltaRot(const Acts::Vector3& deltaR);

}  // namespace tracking::geo

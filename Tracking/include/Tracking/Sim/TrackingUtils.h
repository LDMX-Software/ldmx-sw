#ifndef TRACKUTILS_H_
#define TRACKUTILS_H_

// Recoil back layers numbering scheme for module_

//    +Y  /\   4  3  2  1  0
//        |
//        |
//    -Y  \/   9  8  7  6  5
//          -X <----  ----> +X

// ModN (x_,    y_,   z_)
// 0    (96,   40,  z2)
// 1    (48,   40,  z1)
// 2    (0,    40,  z2)
// 3    (-48,  40,  z1)
// 4    (-96,  40,  z2)

// 5    (96,  -40,  z2)
// 6    (48,  -40,  z1)
// 7    (0,   -40,  z2)
// 8    (-48, -40,  z1)
// 9    (-96, -40,  z2)

// ---< SimCore >---//
#include "SimCore/Event/SimTrackerHit.h"
#include "Tracking/Sim/LdmxSpacePoint.h"

// --- Tracking ---//
#include "Tracking/Event/Track.h"

// --- < ACTS > --- //
#include "Acts/Definitions/Algebra.hpp"
#include "Acts/Definitions/PdgParticle.hpp"
#include "Acts/Definitions/TrackParametrization.hpp"
#include "Acts/Definitions/Units.hpp"
#include "Acts/EventData/TrackParameters.hpp"
#include "Acts/Surfaces/PerigeeSurface.hpp"
#include "Acts/Surfaces/PlaneSurface.hpp"
#include "Acts/Surfaces/Surface.hpp"
#include "Tracking/Event/Measurement.h"
#include "Tracking/Sim/IndexSourceLink.h"

namespace tracking {
namespace sim {
namespace utils {

// This method returns the sensor ID
int getSensorID(const ldmx::SimTrackerHit& hit);

// This method converts a SimHit in a LdmxSpacePoint for the Acts seeder.
//  (1) Rotate the coordinates into acts::seedFinder coordinates defined by
//  B-Field along z_ axis [Z_ldmx -> X_acts, X_ldmx->Y_acts, Y_ldmx->Z_acts]
//  (2) Saves the error information. At the moment the errors are fixed. They
//  should be obtained from the digitized hits_.
// Vol==2 for tagger, Vol==3 for recoil
ldmx::LdmxSpacePoint* convertSimHitToLdmxSpacePoint(
    const ldmx::SimTrackerHit& hit, unsigned int vol = 2, double sigma_u = 0.05,
    double sigma_v = 1.);

// BoundSymMatrix doesn't exist in v36  .. use BoundSquareMatrix
//   have to change this everywhere ..  I think using BoundSysMatrix was defined
//  exactly the same as BoundSquareMatrix is now in ACTs
void flatCov(Acts::BoundSquareMatrix cov, std::vector<double>& v_cov);

Acts::BoundSquareMatrix unpackCov(const std::vector<double>& v_cov);

// Rotate LDMX global -> ACTS frame: z_ldmx->x_acts, x_ldmx->y_acts,
// y_ldmx->z_acts (0 0 1) * (x,y,z)_ldmx = x_acts (1 0 0) * (x,y,z)_ldmx =
// y_acts (0 1 0) * (x,y,z)_ldmx = z_acts
Acts::SquareMatrix3 ldmx2ActsRotation();

Acts::Vector3 ldmx2Acts(Acts::Vector3 ldmx_v);

// Rotate ACTS frame -> LDMX global (inverse of ldmx2Acts, i.e. transpose):
// x_ldmx = y_acts, y_ldmx = z_acts, z_ldmx = x_acts
Acts::SquareMatrix3 acts2LdmxRotation();

Acts::Vector3 acts2Ldmx(Acts::Vector3 acts_v);

// Transform position, momentum and charge to free parameters
Acts::FreeVector toFreeParameters(Acts::Vector3 pos_, Acts::Vector3 mom,
                                  Acts::ActsScalar q);

// Pack the acts track parameters into something that is serializable for the
// event bus
std::vector<double> convertActsToLdmxPars(Acts::BoundVector acts_par);

Acts::BoundVector boundState(const ldmx::Track& trk);

Acts::BoundTrackParameters boundTrackParameters(
    const ldmx::Track& trk, std::shared_ptr<Acts::PerigeeSurface> perigee);

// Return an unbound surface
const std::shared_ptr<Acts::PlaneSurface> unboundSurface(double xloc,
                                                         double yloc = 0.,
                                                         double zloc = 0.);

// This method returns a source link index
std::size_t sourceLinkHash(const Acts::SourceLink& a);

// This method checks if two source links are equal by index
bool sourceLinkEquality(const Acts::SourceLink& a, const Acts::SourceLink& b);

/*
 * Build a TrackState from ACTS BoundTrackParameters.
 * All output quantities (position, momentum, covariance) are in the LDMX
 * global frame: x=horizontal, y=vertical, z=downstream.
 *
 * Covariance transformation steps:
 *   1. Bound (6x6) -> Free (8x8) via ACTS bound-to-free Jacobian
 *   2. Drop time row/col -> 7x7
 *   3. 7D (x,y,z,d0,d1,d2,qop) -> 6D Cartesian (x,y,z,px,py,pz) in ACTS frame
 *   4. Rotate 6x6 covariance ACTS -> LDMX via block-diagonal rotation
 *   5. Flatten upper triangle -> 21-element vector
 */
ldmx::Track::TrackState makeTrackState(
    const Acts::GeometryContext& gctx,
    const Acts::BoundTrackParameters& bound_pars,
    ldmx::TrackStateType ts_type = ldmx::Invalid);

}  // namespace utils
}  // namespace sim
}  // namespace tracking

#endif

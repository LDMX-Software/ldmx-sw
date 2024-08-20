#ifndef TRACKUTILS_H_
#define TRACKUTILS_H_

// TODO:: MAKE A CXX!!

// Recoil back layers numbering scheme for module

//    +Y  /\   4  3  2  1  0
//        |
//        |
//    -Y  \/   9  8  7  6  5
//          -X <----  ----> +X

// ModN (x,    y,   z)
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
#include "Tracking/Event/Measurement.h"
#include "Tracking/Sim/IndexSourceLink.h"


namespace tracking {
namespace sim {
namespace utils {

/*
  It looks like the recoil is subdetector ID 4 and tagger is subdetector ID 1
https://github.com/LDMX-Software/ldmx-sw/blob/0476ccc407e068560518e0614aa83b6eda22e186/DetDescr/include/DetDescr/DetectorID.h#L11-L24
So you could bit shift and mask to get these numbers
https://github.com/LDMX-Software/ldmx-sw/blob/0476ccc407e068560518e0614aa83b6eda22e186/DetDescr/include/DetDescr/DetectorID.h#L38-L39
(sim_tracker_hit.getID() >> 26)&0x3f
or if you are in ldmx-sw, it is easier, more robust, and just as performant to
wrap the ID in the helper class and use its accessors sd =
TrackerID(sim_tracker_hit.getID()).subdet(); if (sd ==
SubdetectorID::SD_TRACKER_RECOIL) {
  // hit in recoil
} else if (sd == SubdetectorID::SD_TRACKER_TAGGER) {
  // hit in tagger
} else {
  // this should never happen since the TrackerID constructor checks for
mal-formed IDs
}
*/

// This method returns the sensor ID
inline int getSensorID(const ldmx::SimTrackerHit& hit) {
  bool debug = false;

  int vol = 2;

  // TODO!! FIX THIS HARDCODE!
  if (hit.getPosition()[2] > 0) vol = 3;

  unsigned int sensorId = 0;
  unsigned int layerId = 0;

  // tagger numbering scheme for surfaces mapping
  // Layers from 1 to 14  => transform to 0->13
  if (vol == 2) {
    sensorId = (hit.getLayerID() + 1) % 2;  // 0,1,0,1 ...

    // v12
    // layerId  = (hit.getLayerID() + 1) / 2; //1,2,3,4,5,6,7
    // v14
    layerId = 7 - ((hit.getLayerID() - 1) / 2);
  }

  // recoil numbering scheme for surfaces mapping
  if (vol == 3) {
    // For axial-stereo modules use the same numbering scheme as the tagger
    if (hit.getLayerID() < 9) {
      sensorId = (hit.getLayerID() + 1) % 2;
      layerId = (hit.getLayerID() + 1) / 2;
    }

    // For the axial only modules
    else {
      sensorId = hit.getModuleID();
      layerId = (hit.getLayerID() + 2) / 2;  // 9->11 /2 = 5 10->12 / 2 = 6
    }
  }

  // vol * 1000 + ly * 100 + sensor
  unsigned int index = vol * 1000 + layerId * 100 + sensorId;

  if (debug) {
    std::cout << "LdmxSpacePointConverter::Check index::" << vol << "--"
              << layerId << "--" << sensorId << "==>" << index << std::endl;
    std::cout << vol << "===" << hit.getLayerID() << "===" << hit.getModuleID()
              << std::endl;
  }

  return index;
}

// This method converts a SimHit in a LdmxSpacePoint for the Acts seeder.
//  (1) Rotate the coordinates into acts::seedFinder coordinates defined by
//  B-Field along z axis [Z_ldmx -> X_acts, X_ldmx->Y_acts, Y_ldmx->Z_acts] (2)
//  Saves the error information. At the moment the errors are fixed. They should
//  be obtained from the digitized hits.

// TODO::Move to shared pointers?!
// TODO::Pass to instances?
// Vol==2 for tagger, Vol==3 for recoil

inline ldmx::LdmxSpacePoint* convertSimHitToLdmxSpacePoint(
    const ldmx::SimTrackerHit& hit, unsigned int vol = 2, double sigma_u = 0.05,
    double sigma_v = 1.) {
  unsigned int index = getSensorID(hit);

  // Rotate position
  float ldmxsp_x = hit.getPosition()[2];
  float ldmxsp_y = hit.getPosition()[0];
  float ldmxsp_z = hit.getPosition()[1];

  return new ldmx::LdmxSpacePoint(ldmxsp_x, ldmxsp_y, ldmxsp_z, hit.getTime(),
                                  index, hit.getEdep(), sigma_u * sigma_u,
                                  sigma_v * sigma_v, hit.getID());
}

// BoundSymMatrix doesn't exist in v36  .. use BoundSquareMatrix
//   have to change this everywhere ..  I think using BoundSysMatrix was defined
//  exactly the same as BoundSquareMatrix is now in ACTs
inline void flatCov(Acts::BoundSquareMatrix cov, std::vector<double>& v_cov) {
  v_cov.clear();
  v_cov.reserve(cov.rows() * (cov.rows() + 1) / 2);
  for (int i = 0; i < cov.rows(); i++)
    for (int j = i; j < cov.cols(); j++) v_cov.push_back(cov(i, j));
}

inline Acts::BoundSquareMatrix unpackCov(const std::vector<double>& v_cov) {
  Acts::BoundSquareMatrix cov;
  int e{0};
  for (int i = 0; i < cov.rows(); i++)
    for (int j = i; j < cov.cols(); j++) {
      cov(i, j) = v_cov.at(e);
      cov(j, i) = cov(i, j);
      e++;
    }

  return cov;
}

// Rotate to ACTS frame
// z->x, x->y, y->z

//(0 0 1) x  = z
//(1 0 0) y  = x
//(0 1 0) z  = y

inline Acts::Vector3 Ldmx2Acts(Acts::Vector3 ldmx_v) {
  // TODO::Move it to a static member
  Acts::SquareMatrix3 acts_rot;
  acts_rot << 0., 0., 1., 1., 0., 0., 0., 1, 0.;

  return acts_rot * ldmx_v;
}

// Transform position, momentum and charge to free parameters

inline Acts::FreeVector toFreeParameters(Acts::Vector3 pos, Acts::Vector3 mom,
                                         Acts::ActsScalar q) {
  Acts::FreeVector free_params;
  Acts::ActsScalar p = mom.norm() * Acts::UnitConstants::MeV;

  free_params[Acts::eFreePos0] = pos(Acts::ePos0) * Acts::UnitConstants::mm;
  free_params[Acts::eFreePos1] = pos(Acts::ePos1) * Acts::UnitConstants::mm;
  free_params[Acts::eFreePos2] = pos(Acts::ePos2) * Acts::UnitConstants::mm;
  free_params[Acts::eFreeTime] = 0.;
  free_params[Acts::eFreeDir0] = mom(0) / mom.norm();
  free_params[Acts::eFreeDir1] = mom(1) / mom.norm();
  free_params[Acts::eFreeDir2] = mom(2) / mom.norm();
  free_params[Acts::eFreeQOverP] =
      (q != Acts::ActsScalar(0)) ? (q / p) : 0.;  // 1. / p instead?

  return free_params;
}

// Pack the acts track parameters into something that is serializable for the
// event bus

inline std::vector<double> convertActsToLdmxPars(Acts::BoundVector acts_par) {
  std::vector<double> v_ldmx(
      acts_par.data(), acts_par.data() + acts_par.rows() * acts_par.cols());
  return v_ldmx;
}

inline Acts::BoundVector boundState(const ldmx::Track& trk) {
  Acts::BoundVector paramVec;
  paramVec << trk.getD0(), trk.getZ0(), trk.getPhi(), trk.getTheta(),
      trk.getQoP(), trk.getT();
  return paramVec;
}

inline Acts::BoundVector boundState(const ldmx::Track::TrackState& ts) {
  Acts::BoundVector paramVec;
  paramVec << ts.params[0], ts.params[1], ts.params[2], ts.params[3],
      ts.params[4], ts.params[5];
  return paramVec;
}

inline Acts::BoundTrackParameters boundTrackParameters(
    const ldmx::Track& trk, std::shared_ptr<Acts::PerigeeSurface> perigee) {
  Acts::BoundVector paramVec = boundState(trk);
  Acts::BoundSquareMatrix covMat = unpackCov(trk.getPerigeeCov());
  auto partHypo{Acts::SinglyChargedParticleHypothesis::electron()};
  //  auto
  //  part{Acts::GenericParticleHypothesis(Acts::ParticleHypothesis(Acts::PdgParticle(trk.getPdgID())))};
  //  return Acts::BoundTrackParameters(perigee, paramVec, std::move(covMat));
  // need to add particle hypothesis
  return Acts::BoundTrackParameters(perigee, paramVec, std::move(covMat),
                                    partHypo);
}

inline Acts::BoundTrackParameters btp(const ldmx::Track::TrackState& ts,
                                      std::shared_ptr<Acts::Surface> surf,
                                      int pdgid) {
  Acts::BoundVector paramVec = boundState(ts);
  Acts::BoundSquareMatrix covMat = unpackCov(ts.cov);
  auto partHypo{Acts::SinglyChargedParticleHypothesis::electron()};
  //  auto
  //  part{Acts::GenericParticleHypothesis(Acts::ParticleHypothesis(Acts::PdgParticle(pdgid)))};
  return Acts::BoundTrackParameters(surf, paramVec, std::move(covMat),
                                    partHypo);
}

// Return an unbound surface
inline const std::shared_ptr<Acts::PlaneSurface> unboundSurface(
    double xloc, double yloc = 0., double zloc = 0.) {
  // Define the target surface - be careful:
  //  x - downstream
  //  y - left (when looking along x)
  //  z - up
  //  Passing identity here means that your target surface is oriented in the
  //  same way
  Acts::RotationMatrix3 surf_rotation = Acts::RotationMatrix3::Zero();
  // u direction along +Y
  surf_rotation(1, 0) = 1;
  // v direction along +Z
  surf_rotation(2, 1) = 1;
  // w direction along +X
  surf_rotation(0, 2) = 1;

  Acts::Vector3 pos(xloc, yloc, zloc);
  Acts::Translation3 surf_translation(pos);
  Acts::Transform3 surf_transform(surf_translation * surf_rotation);

  // Unbounded surface
  const std::shared_ptr<Acts::PlaneSurface> target_surface =
      Acts::Surface::makeShared<Acts::PlaneSurface>(surf_transform);

  return Acts::Surface::makeShared<Acts::PlaneSurface>(surf_transform);
}

// This method returns a source link index
inline std::size_t sourceLinkHash(const Acts::SourceLink& a) { 
  return static_cast<std::size_t>(
      a.get<ActsExamples::IndexSourceLink>().index());
    }

// This method checks if two source links are equal by index
inline bool sourceLinkEquality(const Acts::SourceLink& a, const Acts::SourceLink& b) {
  return a.get<ActsExamples::IndexSourceLink>().index() ==
         b.get<ActsExamples::IndexSourceLink>().index();
}

}  // namespace utils
}  // namespace sim
}  // namespace tracking

#endif

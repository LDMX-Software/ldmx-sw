#include "Tracking/Sim/TrackingUtils.h"

#include "DetDescr/TrackerID.h"

namespace tracking {
namespace sim {
namespace utils {

// This method returns the sensor ID
int getSensorID(const ldmx::SimTrackerHit& hit) {
  bool debug = false;

  ldmx::TrackerID tid(hit.getID());
  int vol = (tid.subdet() == ldmx::SD_TRACKER_RECOIL) ? 3 : 2;

  unsigned int sensor_id = 0;
  unsigned int layer_id = 0;

  // tagger numbering scheme for surfaces mapping
  // Layers from 1 to 14  => transform to 0->13
  if (vol == 2) {
    sensor_id = (hit.getLayerID() + 1) % 2;  // 0,1,0,1 ...

    // v12
    // layerId  = (hit.getLayerID() + 1) / 2; //1,2,3,4,5,6,7
    // v14
    layer_id = 7 - ((hit.getLayerID() - 1) / 2);
  }

  // recoil numbering scheme for surfaces mapping
  if (vol == 3) {
    // For axial-stereo modules use the same numbering scheme as the tagger
    if (hit.getLayerID() < 9) {
      sensor_id = (hit.getLayerID() + 1) % 2;
      layer_id = (hit.getLayerID() + 1) / 2;
    }

    // For the axial only modules
    else {
      sensor_id = hit.getModuleID();
      layer_id = (hit.getLayerID() + 2) / 2;  // 9->11 /2 = 5 10->12 / 2 = 6
    }
  }

  // vol * 1000 + ly * 100 + sensor
  unsigned int index = vol * 1000 + layer_id * 100 + sensor_id;

  if (debug) {
    std::cout << "LdmxSpacePointConverter::Check index::" << vol << "--"
              << layer_id << "--" << sensor_id << "==>" << index << std::endl;
    std::cout << vol << "===" << hit.getLayerID() << "===" << hit.getModuleID()
              << std::endl;
  }

  return index;
}

// This method converts a SimHit in a LdmxSpacePoint for the Acts seeder.
//  (1) Rotate the coordinates into acts::seedFinder coordinates defined by
//  B-Field along z_ axis [Z_ldmx -> X_acts, X_ldmx->Y_acts, Y_ldmx->Z_acts]
//  (2) Saves the error information. At the moment the errors are fixed. They
//  should be obtained from the digitized hits_.
// Vol==2 for tagger, Vol==3 for recoil
ldmx::LdmxSpacePoint* convertSimHitToLdmxSpacePoint(
    const ldmx::SimTrackerHit& hit, unsigned int vol, double sigma_u,
    double sigma_v) {
  unsigned int index = getSensorID(hit);

  // Rotate position
  float ldmxsp_x = hit.getPosition()[2];
  float ldmxsp_y = hit.getPosition()[0];
  float ldmxsp_z = hit.getPosition()[1];

  return new ldmx::LdmxSpacePoint(ldmxsp_x, ldmxsp_y, ldmxsp_z, hit.getTime(),
                                  index, hit.getEdep(), sigma_u * sigma_u,
                                  sigma_v * sigma_v, hit.getID());
}

void flatCov(Acts::BoundSquareMatrix cov, std::vector<double>& v_cov) {
  v_cov.clear();
  v_cov.reserve(cov.rows() * (cov.rows() + 1) / 2);
  for (int i = 0; i < cov.rows(); i++)
    for (int j = i; j < cov.cols(); j++) v_cov.push_back(cov(i, j));
}

Acts::BoundSquareMatrix unpackCov(const std::vector<double>& v_cov) {
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

// Rotate LDMX global -> ACTS frame: z_ldmx->x_acts, x_ldmx->y_acts,
// y_ldmx->z_acts (0 0 1) * (x,y,z)_ldmx = x_acts (1 0 0) * (x,y,z)_ldmx =
// y_acts (0 1 0) * (x,y,z)_ldmx = z_acts
Acts::SquareMatrix3 ldmx2ActsRotation() {
  Acts::SquareMatrix3 r;
  r << 0., 0., 1., 1., 0., 0., 0., 1., 0.;
  return r;
}

Acts::Vector3 ldmx2Acts(Acts::Vector3 ldmx_v) {
  return ldmx2ActsRotation() * ldmx_v;
}

// Rotate ACTS frame -> LDMX global (inverse of ldmx2Acts, i.e. transpose):
// x_ldmx = y_acts, y_ldmx = z_acts, z_ldmx = x_acts
Acts::SquareMatrix3 acts2LdmxRotation() {
  return ldmx2ActsRotation().transpose();
}

Acts::Vector3 acts2Ldmx(Acts::Vector3 acts_v) {
  return acts2LdmxRotation() * acts_v;
}

// Transform position, momentum and charge to free parameters
Acts::FreeVector toFreeParameters(Acts::Vector3 pos_, Acts::Vector3 mom,
                                  Acts::ActsScalar q) {
  Acts::FreeVector free_params;
  Acts::ActsScalar p = mom.norm() * Acts::UnitConstants::MeV;

  free_params[Acts::eFreePos0] = pos_(Acts::ePos0) * Acts::UnitConstants::mm;
  free_params[Acts::eFreePos1] = pos_(Acts::ePos1) * Acts::UnitConstants::mm;
  free_params[Acts::eFreePos2] = pos_(Acts::ePos2) * Acts::UnitConstants::mm;
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
std::vector<double> convertActsToLdmxPars(Acts::BoundVector acts_par) {
  std::vector<double> v_ldmx(
      acts_par.data(), acts_par.data() + acts_par.rows() * acts_par.cols());
  return v_ldmx;
}

Acts::BoundVector boundState(const ldmx::Track& trk) {
  Acts::BoundVector param_vec;
  param_vec << trk.getD0(), trk.getZ0(), trk.getPhi(), trk.getTheta(),
      trk.getQoP(), trk.getT();
  return param_vec;
}

Acts::BoundTrackParameters boundTrackParameters(
    const ldmx::Track& trk, std::shared_ptr<Acts::PerigeeSurface> perigee) {
  Acts::BoundVector param_vec = boundState(trk);
  Acts::BoundSquareMatrix cov_mat = unpackCov(trk.getPerigeeCov());
  auto part_hypo{Acts::SinglyChargedParticleHypothesis::electron()};
  return Acts::BoundTrackParameters(perigee, param_vec, std::move(cov_mat),
                                    part_hypo);
}

// Return an unbound surface
const std::shared_ptr<Acts::PlaneSurface> unboundSurface(
    double xloc, double yloc, double zloc) {
  // Define the target surface - be careful:
  //  x_ - downstream
  //  y_ - left (when looking along x_)
  //  z_ - up
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
std::size_t sourceLinkHash(const Acts::SourceLink& a) {
  return static_cast<std::size_t>(
      a.get<acts_examples::IndexSourceLink>().index());
}

// This method checks if two source links are equal by index
bool sourceLinkEquality(const Acts::SourceLink& a,
                        const Acts::SourceLink& b) {
  return a.get<acts_examples::IndexSourceLink>().index() ==
         b.get<acts_examples::IndexSourceLink>().index();
}

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
    ldmx::TrackStateType ts_type) {
  ldmx::Track::TrackState new_ts;
  new_ts.ts_type_ = ts_type;

  const double p = bound_pars.absoluteMomentum();  // GeV
  const Acts::Vector3 acts_pos = bound_pars.position(gctx);
  const Acts::Vector3 acts_dir = bound_pars.direction();

  // Rotate position and momentum to LDMX frame
  const Acts::SquareMatrix3 r = acts2LdmxRotation();
  const Acts::Vector3 ldmx_pos = r * acts_pos;
  const Acts::Vector3 ldmx_mom = r * (acts_dir * p);

  new_ts.pos_ = {ldmx_pos[0], ldmx_pos[1], ldmx_pos[2]};
  // Convert momentum from ACTS native units (GeV) to MeV
  new_ts.mom_ = {ldmx_mom[0] / Acts::UnitConstants::MeV,
                 ldmx_mom[1] / Acts::UnitConstants::MeV,
                 ldmx_mom[2] / Acts::UnitConstants::MeV};

  const auto& bound_cov = bound_pars.covariance();
  if (!bound_cov.has_value()) {
    std::cerr << "TrackingUtils::makeTrackState: bound covariance missing\n";
    return new_ts;
  }

  // Step 1: Bound (6x6) -> Free (8x8) covariance
  const Acts::BoundToFreeMatrix j_btf =
      bound_pars.referenceSurface().boundToFreeJacobian(gctx, acts_pos,
                                                        acts_dir);
  const Acts::FreeSquareMatrix free_cov =
      j_btf * bound_cov.value() * j_btf.transpose();

  // Step 2: Drop time row/col (eFreeTime = 3) -> 7x7
  // Remaining indices: pos(0,1,2), dir(4,5,6), qop(7)
  constexpr std::array<int, 7> k_keep = {
      Acts::eFreePos0, Acts::eFreePos1, Acts::eFreePos2,  Acts::eFreeDir0,
      Acts::eFreeDir1, Acts::eFreeDir2, Acts::eFreeQOverP};
  Eigen::Matrix<double, 7, 7> free_cov7;
  for (int i = 0; i < 7; ++i)
    for (int j = 0; j < 7; ++j)
      free_cov7(i, j) = free_cov(k_keep[i], k_keep[j]);

  // Step 3: Jacobian from 7D free-no-time -> 6D Cartesian in ACTS frame
  // p_i = dir_i * p,  dp_i/d(dir_j) = p*delta_ij,  dp_i/d(qop) = -dir_i*p/qop
  const double qop = bound_pars.parameters()[Acts::eBoundQOverP];
  Eigen::Matrix<double, 6, 7> j_fp = Eigen::Matrix<double, 6, 7>::Zero();
  j_fp.block<3, 3>(0, 0) = Eigen::Matrix3d::Identity();      // pos -> pos
  j_fp.block<3, 3>(3, 3) = p * Eigen::Matrix3d::Identity();  // dir -> mom
  j_fp(3, 6) = -acts_dir[0] * p / qop;                       // qop -> px
  j_fp(4, 6) = -acts_dir[1] * p / qop;                       // qop -> py
  j_fp(5, 6) = -acts_dir[2] * p / qop;                       // qop -> pz

  const Eigen::Matrix<double, 6, 6> cov_acts =
      j_fp * free_cov7 * j_fp.transpose();

  // Step 4: Rotate covariance ACTS -> LDMX using block-diagonal R_6 = diag(R,R)
  Eigen::Matrix<double, 6, 6> r_6 = Eigen::Matrix<double, 6, 6>::Zero();
  r_6.block<3, 3>(0, 0) = r;
  r_6.block<3, 3>(3, 3) = r;
  const Eigen::Matrix<double, 6, 6> cov_ldmx = r_6 * cov_acts * r_6.transpose();

  // Step 5: Flatten upper triangle -> 21 elements.
  // Scale momentum rows/cols from GeV to MeV:
  //   pos-pos (i<3, j<3): x1       [mm^2]
  //   pos-mom (i<3, j>=3): x1000   [mm*MeV]
  //   mom-mom (i>=3, j>=3): x1e6   [MeV^2]
  const double mev = Acts::UnitConstants::MeV;
  new_ts.pos_mom_cov_.reserve(21);
  for (int i = 0; i < 6; ++i) {
    for (int j = i; j < 6; ++j) {
      double scale = 1.0;
      if (i >= 3) scale /= mev;  // row is momentum (GeV -> MeV)
      if (j >= 3) scale /= mev;  // col is momentum (GeV -> MeV)
      new_ts.pos_mom_cov_.push_back(cov_ldmx(i, j) * scale);
    }
  }

  return new_ts;
}

}  // namespace utils
}  // namespace sim
}  // namespace tracking

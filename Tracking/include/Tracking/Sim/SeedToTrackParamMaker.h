#pragma once

#include "Acts/Definitions/Algebra.hpp"
#include "Acts/Definitions/TrackParametrization.hpp"
// #include "Acts/Utilities/VectorHelpers.hpp"
#include <optional>

#include "Acts/Definitions/Units.hpp"
#include "Acts/Utilities/Helpers.hpp"

namespace tracking {
namespace sim {

class SeedToTrackParamMaker {
 public:
  SeedToTrackParamMaker() {};

  template <typename external_spacepoint_t>
  bool karimakiFit(const std::vector<external_spacepoint_t*>& sp,
                   std::array<double, 9>& data, const Acts::Vector2 refPoint);

  // We assume that a track propagates from point r1 to point r2
  // If the test function returns negative means that we need to transform
  // rho -> -rho
  // phi -> phi+pi,
  // d->-d

  bool transformRhoPhid(const Acts::Vector2& r1, const Acts::Vector2& r2,
                        double& phi, double& rho, double& d) {
    float track_dir = cos(phi) * (r1[0] - r2[0]) + sin(phi) * (r1[1] - r2[1]);

    if (track_dir < 0) {
      phi = M_PI + phi;
      rho = -rho;
      d = -d;
      return true;
    }

    else
      return false;
  }

  /// This resembles the method used in ATLAS for the seed fitting
  /// L811
  /// https://acode-browser.usatlas.bnl.gov/lxr/source/athena/InnerDetector/InDetRecTools/SiTrackMakerTool_xk/src/SiTrackMaker_xk.cxx
  // Acts::Seed<T> removed in v47 — fitSeedAtlas(Seed) unused, commented out
  // template <typename external_spacepoint_t>
  // bool fitSeedAtlas(const Acts::Seed<external_spacepoint_t>& seed,
  //                   std::array<double, 9>& data, const Acts::Transform3& Tp,
  //                   const double& bFieldZ);

  template <typename external_spacepoint_t>
  bool fitSeedAtlas(const std::vector<external_spacepoint_t>& sp,
                    std::array<double, 9>& data, const Acts::Transform3& Tp,
                    const double& bFieldZ);

  /// This is a simple Line and Parabola fit (from HPS reconstruction by Robert
  /// Johnson)
  // Acts::Seed<T> removed in v47 — fitSeedLinPar(Seed) unused, commented out
  // template <typename external_spacepoint_t>
  // bool fitSeedLinPar(const Acts::Seed<external_spacepoint_t>& seed,
  //                    std::vector<double>& data);

  /// Estimate the full track parameters from three space points
  ///
  /// This method is based on the conformal map transformation. It estimates the
  /// full bound track parameters, i.e. (loc0, loc1, phi, theta, q/p, t) at the
  /// bottom space point. The bottom space is assumed to be the first element
  /// in the range defined by the iterators. The magnetic
  /// field (which might be along any direction) is also necessary for the
  /// momentum estimation.
  ///
  /// It resembles the method used in ATLAS for the track parameters
  /// estimated from seed, i.e. the function InDet::SiTrackMaker_xk::getAtaPlane
  /// here:
  /// https://acode-browser.usatlas.bnl.gov/lxr/source/athena/InnerDetector/InDetRecTools/SiTrackMakerTool_xk/src/SiTrackMaker_xk.cxx
  ///
  /// @tparam spacepoint_iterator_t  The type of space point iterator
  ///
  /// @param tp the local to global transformation
  /// @param spBegin is the begin iterator for the space points
  /// @param spEnd is the end iterator for the space points
  /// @param surface is the surface of the bottom space point. The estimated
  /// bound track parameters will be represented also at this surface
  /// @param bField is the magnetic field vector
  /// @param bFieldMin is the minimum magnetic field required to trigger the
  /// estimation of q/pt
  /// @param mass is the estimated particle mass
  ///
  /// @return optional bound parameters

  template <typename spacepoint_iterator_t>
  std::optional<Acts::BoundVector> estimateTrackParamsFromSeed(
      const Acts::Transform3& Tp, spacepoint_iterator_t spBegin,
      spacepoint_iterator_t spEnd, Acts::Vector3 bField,
      double bFieldMin,
      double mass = 139.57018 * Acts::UnitConstants::MeV) {
    // Check the number of provided space points
    size_t num_sp = std::distance(spBegin, spEnd);
    if (num_sp != 3) {
      std::cout << "ERROR::less than 3 point provided" << std::endl;
      return std::nullopt;
    }

    // Convert bField to Tesla
    double b_field_in_tesla = bField.norm() / Acts::UnitConstants::T;
    double b_field_min_in_tesla = bFieldMin / Acts::UnitConstants::T;
    // Check if magnetic field is too small
    if (b_field_in_tesla < b_field_min_in_tesla) {
      // @todo shall we use straight-line estimation and use default q/pt in
      // such case?
      std::cout << "The magnetic field at the bottom space point: B = "
                << b_field_in_tesla
                << " T is smaller than |B|_min = " << b_field_min_in_tesla
                << " T. Estimation is not performed." << std::endl;
      return std::nullopt;
    }

    // The global positions of the bottom, middle and space points
    std::array<Acts::Vector3, 3> sp_global_positions = {
        Acts::Vector3::Zero(), Acts::Vector3::Zero(), Acts::Vector3::Zero()};
    // The first, second and third space point are assumed to be bottom, middle
    // and top space point, respectively
    for (size_t isp = 0; isp < 3; ++isp) {
      spacepoint_iterator_t it = std::next(spBegin, isp);
      if (*it == nullptr) {
        std::cout << "Empty space point found. This should not happen."
                  << std::endl;
        return std::nullopt;
      }
      const auto& sp = *it;
      sp_global_positions[isp] = Acts::Vector3(sp->x(), sp->y(), sp->z());
    }

    // Define a new coordinate frame with its origin at the bottom space point,
    // z_ axis along the magnetic field direction and y_ axis perpendicular to
    // vector from the bottom to middle space point. Hence, the projection of
    // the middle space point on the tranverse plane will be located at the x_
    // axis of the new frame.
    Acts::Vector3 rel_vec = sp_global_positions[1] - sp_global_positions[0];
    Acts::Vector3 new_z_axis = bField.normalized();
    Acts::Vector3 new_y_axis = new_z_axis.cross(rel_vec).normalized();
    Acts::Vector3 new_x_axis = new_y_axis.cross(new_z_axis);
    Acts::RotationMatrix3 rotation;
    rotation.col(0) = new_x_axis;
    rotation.col(1) = new_y_axis;
    rotation.col(2) = new_z_axis;
    // The center of the new frame is at the bottom space point
    Acts::Translation3 trans(sp_global_positions[0]);
    // The transform which constructs the new frame
    Acts::Transform3 transform(trans * rotation);

    // The coordinate of the middle and top space point in the new frame
    Acts::Vector3 local1 = transform.inverse() * sp_global_positions[1];
    Acts::Vector3 local2 = transform.inverse() * sp_global_positions[2];

    // Lambda to transform the coordinates to the (u, v) space
    auto uv_transform = [](const Acts::Vector3& local) -> Acts::Vector2 {
      Acts::Vector2 uv;
      double denominator =
          local.x() * local.x() + local.y() * local.y();
      uv.x() = local.x() / denominator;
      uv.y() = local.y() / denominator;
      return uv;
    };
    // The uv1.y() should be zero
    Acts::Vector2 uv1 = uv_transform(local1);
    Acts::Vector2 uv2 = uv_transform(local2);

    // A,B are slope and intercept of the straight line in the u,v plane
    // connecting the three points
    double a = (uv2.y() - uv1.y()) / (uv2.x() - uv1.x());
    double b = uv2.y() - a * uv2.x();
    // Curvature (with a sign) estimate
    double rho = -2.0 * b / std::hypot(1., a);
    // The projection of the top space point on the transverse plane of the new
    // frame
    double rn = local2.x() * local2.x() + local2.y() * local2.y();
    // The (1/tanTheta) of momentum in the new frame,
    double inv_tan_theta =
        local2.z() * std::sqrt(1. / rn) / (1. + rho * rho * rn);
    // The momentum direction in the new frame (the center of the circle has the
    // coordinate (-1.*A/(2*B), 1./(2*B)))
    Acts::Vector3 trans_direction(1., a, std::hypot(1, a) * inv_tan_theta);
    // Transform it back to the original frame
    [[maybe_unused]] Acts::Vector3 direction =
        rotation * trans_direction.normalized();

    // Initialize the bound parameters vector
    Acts::BoundVector params = Acts::BoundVector::Zero();

    // The estimated phi and theta
    // params[Acts::eBoundPhi] = Acts::VectorHelpers::phi(direction);
    // params[Acts::eBoundTheta] = Acts::VectorHelpers::theta(direction);

    Acts::Vector3 bottom_local_pos = Tp.inverse() * sp_global_positions[0];

    // The estimated loc0 and loc1
    params[Acts::eBoundLoc0] = bottom_local_pos.x();
    params[Acts::eBoundLoc1] = bottom_local_pos.y();

    // The estimated q/pt in [GeV/c]^-1 (note that the pt is the projection of
    // momentum on the transverse plane of the new frame)
    double q_over_pt =
        rho * (Acts::UnitConstants::m) / (0.3 * b_field_in_tesla);
    // The estimated q/p in [GeV/c]^-1
    params[Acts::eBoundQOverP] = q_over_pt / std::hypot(1., inv_tan_theta);

    // The estimated momentum, and its projection along the magnetic field
    // diretion
    double p_in_ge_v = std::abs(1.0 / params[Acts::eBoundQOverP]);
    double pz_in_ge_v = 1.0 / std::abs(q_over_pt) * inv_tan_theta;
    double mass_in_ge_v = mass / Acts::UnitConstants::GeV;
    // The estimated velocity, and its projection along the magnetic field
    // diretion
    double v = p_in_ge_v / std::hypot(p_in_ge_v, mass_in_ge_v);
    double vz = pz_in_ge_v / std::hypot(p_in_ge_v, mass_in_ge_v);
    // The z_ coordinate of the bottom space point along the magnetic field
    // direction
    double pathz = sp_global_positions[0].dot(bField) / bField.norm();
    // The estimated time (use path length along magnetic field only if it's not
    // zero)
    if (pathz != 0) {
      params[Acts::eBoundTime] = pathz / vz;
    } else {
      params[Acts::eBoundTime] = sp_global_positions[0].norm() / v;
    }

    return params;
  }
};
}  // namespace sim
}  // namespace tracking

#include "SeedToTrackParamMaker.ipp"

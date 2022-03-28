#pragma once

#include "Acts/Seeding/Seed.hpp"
#include "Acts/Definitions/Algebra.hpp"
#include "Acts/Definitions/TrackParametrization.hpp"
#include "Acts/Definitions/Units.hpp"
#include "Acts/Utilities/Helpers.hpp"


namespace tracking{
namespace sim{

class SeedToTrackParamMaker {
 public:
  
  SeedToTrackParamMaker(){};
  
  template <typename external_spacepoint_t>
  bool KarimakiFit(const std::vector<external_spacepoint_t*>&sp, std::array<double,9>& data, const Acts::Vector2 refPoint);
  
  //We assume that a track propagates from point r1 to point r2
  //If the test function returns negative means that we need to transform 
  //rho -> -rho 
  //phi -> phi+pi, 
  //d->-d
  
  bool transformRhoPhid(const Acts::Vector2 &r1, const Acts::Vector2 &r2, 
                        double &phi,double& rho, double& d) {
            
    float trackDir = cos(phi)*(r1[0] - r2[0]) + sin(phi)*(r1[1]-r2[1]);
            
    if (trackDir < 0) {
      phi = M_PI + phi;
      rho = -rho;
      d   = -d;
      return true;
    }
            
    else
      return false;

  }

  /// This resembles the method used in ATLAS for the seed fitting
  /// L811 https://acode-browser.usatlas.bnl.gov/lxr/source/athena/InnerDetector/InDetRecTools/SiTrackMakerTool_xk/src/SiTrackMaker_xk.cxx
  template <typename external_spacepoint_t>
  bool FitSeedAtlas(const Acts::Seed<external_spacepoint_t>& seed, std::array<double, 9>& data, const Acts::Transform3& Tp, const double& bFieldZ);
  
  template <typename external_spacepoint_t>
  bool FitSeedAtlas(const std::vector<external_spacepoint_t>& sp, std::array<double,9>& data, const Acts::Transform3& Tp, const double& bFieldZ);
  
  /// This is a simple Line and Parabola fit (from HPS reconstruction by Robert Johnson)
  template <typename external_spacepoint_t>
  bool FitSeedLinPar(const Acts::Seed<external_spacepoint_t>& seed, std::vector<double>& data);



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
  /// @param surface is the surface of the bottom space point. The estimated bound
  /// track parameters will be represented also at this surface
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
      Acts::ActsScalar bFieldMin, Acts::ActsScalar mass = 139.57018 * Acts::UnitConstants::MeV) {
  
    // Check the number of provided space points
    size_t numSP = std::distance(spBegin, spEnd);
    if (numSP != 3) {
      std::cout<<"ERROR::less than 3 point provided"<<std::endl;
      return std::nullopt;
    }

    // Convert bField to Tesla
    Acts::ActsScalar bFieldInTesla = bField.norm() / Acts::UnitConstants::T;
    Acts::ActsScalar bFieldMinInTesla = bFieldMin / Acts::UnitConstants::T;
    // Check if magnetic field is too small
    if (bFieldInTesla < bFieldMinInTesla) {
      // @todo shall we use straight-line estimation and use default q/pt in such
      // case?
      std::cout<<"The magnetic field at the bottom space point: B = "
               << bFieldInTesla << " T is smaller than |B|_min = "
               << bFieldMinInTesla << " T. Estimation is not performed."<<std::endl;
      return std::nullopt;
    }

    // The global positions of the bottom, middle and space points
    std::array<Acts::Vector3, 3> spGlobalPositions = {Acts::Vector3::Zero(), Acts::Vector3::Zero(),
      Acts::Vector3::Zero()};
    // The first, second and third space point are assumed to be bottom, middle
    // and top space point, respectively
    for (size_t isp = 0; isp < 3; ++isp) {
      spacepoint_iterator_t it = std::next(spBegin, isp);
      if (*it == nullptr) {
        std::cout<<"Empty space point found. This should not happen."<<std::endl;
        return std::nullopt;
      }
      const auto& sp = *it;
      spGlobalPositions[isp] = Acts::Vector3(sp->x(), sp->y(), sp->z());
    }

    // Define a new coordinate frame with its origin at the bottom space point, z
    // axis along the magnetic field direction and y axis perpendicular to vector
    // from the bottom to middle space point. Hence, the projection of the middle
    // space point on the tranverse plane will be located at the x axis of the new
    // frame.
    Acts::Vector3 relVec = spGlobalPositions[1] - spGlobalPositions[0];
    Acts::Vector3 newZAxis = bField.normalized();
    Acts::Vector3 newYAxis = newZAxis.cross(relVec).normalized();
    Acts::Vector3 newXAxis = newYAxis.cross(newZAxis);
    Acts::RotationMatrix3 rotation;
    rotation.col(0) = newXAxis;
    rotation.col(1) = newYAxis;
    rotation.col(2) = newZAxis;
    // The center of the new frame is at the bottom space point
    Acts::Translation3 trans(spGlobalPositions[0]);
    // The transform which constructs the new frame
    Acts::Transform3 transform(trans * rotation);

    // The coordinate of the middle and top space point in the new frame
    Acts::Vector3 local1 = transform.inverse() * spGlobalPositions[1];
    Acts::Vector3 local2 = transform.inverse() * spGlobalPositions[2];

    // Lambda to transform the coordinates to the (u, v) space
    auto uvTransform = [](const Acts::Vector3& local) -> Acts::Vector2 {
      Acts::Vector2 uv;
      Acts::ActsScalar denominator = local.x() * local.x() + local.y() * local.y();
      uv.x() = local.x() / denominator;
      uv.y() = local.y() / denominator;
      return uv;
    };
    // The uv1.y() should be zero
    Acts::Vector2 uv1 = uvTransform(local1);
    Acts::Vector2 uv2 = uvTransform(local2);

    // A,B are slope and intercept of the straight line in the u,v plane
    // connecting the three points
    Acts::ActsScalar A = (uv2.y() - uv1.y()) / (uv2.x() - uv1.x());
    Acts::ActsScalar B = uv2.y() - A * uv2.x();
    // Curvature (with a sign) estimate
    Acts::ActsScalar rho = -2.0 * B / std::hypot(1., A);
    // The projection of the top space point on the transverse plane of the new
    // frame
    Acts::ActsScalar rn = local2.x() * local2.x() + local2.y() * local2.y();
    // The (1/tanTheta) of momentum in the new frame,
    Acts::ActsScalar invTanTheta =
        local2.z() * std::sqrt(1. / rn) / (1. + rho * rho * rn);
    // The momentum direction in the new frame (the center of the circle has the
    // coordinate (-1.*A/(2*B), 1./(2*B)))
    Acts::Vector3 transDirection(1., A, std::hypot(1, A) * invTanTheta);
    // Transform it back to the original frame
    Acts::Vector3 direction = rotation * transDirection.normalized();

    // Initialize the bound parameters vector
    Acts::BoundVector params = Acts::BoundVector::Zero();

    // The estimated phi and theta
    params[Acts::eBoundPhi] = Acts::VectorHelpers::phi(direction);
    params[Acts::eBoundTheta] = Acts::VectorHelpers::theta(direction);

    Acts::Vector3 bottomLocalPos = Tp.inverse() * spGlobalPositions[0];
        
    // The estimated loc0 and loc1
    params[Acts::eBoundLoc0] = bottomLocalPos.x();
    params[Acts::eBoundLoc1] = bottomLocalPos.y();

    // The estimated q/pt in [GeV/c]^-1 (note that the pt is the projection of
    // momentum on the transverse plane of the new frame)
    Acts::ActsScalar qOverPt = rho * (Acts::UnitConstants::m) / (0.3 * bFieldInTesla);
    // The estimated q/p in [GeV/c]^-1
    params[Acts::eBoundQOverP] = qOverPt / std::hypot(1., invTanTheta);

    // The estimated momentum, and its projection along the magnetic field
    // diretion
    Acts::ActsScalar pInGeV = std::abs(1.0 / params[Acts::eBoundQOverP]);
    Acts::ActsScalar pzInGeV = 1.0 / std::abs(qOverPt) * invTanTheta;
    Acts::ActsScalar massInGeV = mass / Acts::UnitConstants::GeV;
    // The estimated velocity, and its projection along the magnetic field
    // diretion
    Acts::ActsScalar v = pInGeV / std::hypot(pInGeV, massInGeV);
    Acts::ActsScalar vz = pzInGeV / std::hypot(pInGeV, massInGeV);
    // The z coordinate of the bottom space point along the magnetic field
    // direction
    Acts::ActsScalar pathz = spGlobalPositions[0].dot(bField) / bField.norm();
    // The estimated time (use path length along magnetic field only if it's not
    // zero)
    if (pathz != 0) {
      params[Acts::eBoundTime] = pathz / vz;
    } else {
      params[Acts::eBoundTime] = spGlobalPositions[0].norm() / v;
    }

    return params;
  }
  
};
}
}

#include "SeedToTrackParamMaker.ipp"

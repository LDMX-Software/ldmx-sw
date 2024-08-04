#pragma once

#include "Acts/Definitions/Algebra.hpp"

class GaussianDistribution2D {
 public:
  GaussianDistribution2D(double normalization, const Acts::Vector3& mean,
                         const Acts::Vector3& major_axis,
                         const Acts::Vector3& minor_axis);

  void transform(const Acts::Transform3& transform);
  GaussianDistribution2D transformed(const Acts::Transform3& transform);

  double getNormalization() { return normalization_; };
  Acts::Vector3 getMean() { return mean_; };
  double sigma1D(const Acts::Vector3& axis);
  double covxy(const Acts::Vector3& xaxis, const Acts::Vector3 yaxis);
  double upperIntegral1D(const Acts::Vector3& axis, double integration_limit);

 private:
  double normalization_{1.0};
  Acts::Vector3 mean_{0., 0., 0.};
  Acts::Vector3 major_axis_{0., 0., 0.};
  Acts::Vector3 minor_axis_{0., 0., 0.};
};

#include "Tracking/Digitization/GaussianDistribution2D.h"
#include <iostream>

GaussianDistribution2D::GaussianDistribution2D(double normalization,
                                               const Acts::Vector3& mean,
                                               const Acts::Vector3& major_axis,
                                               const Acts::Vector3& minor_axis) {

  normalization_ = normalization;
  mean_ = mean;
  
  if (std::abs(major_axis.dot(minor_axis)) < 1.e-9) {
    major_axis_ = major_axis;
    minor_axis_ = minor_axis;
  }
  else {
    std::cout<<__PRETTY_FUNCTION__<<"FAILURE: Axes are not perpendicular"<<std::endl;
  }
  
}


void GaussianDistribution2D::transform(const Acts::Transform3& transform) {
  
  mean_       = transform * mean_;
  major_axis_ = transform * major_axis_;
  minor_axis_ = transform * minor_axis_;
  
}


GaussianDistribution2D GaussianDistribution2D::transformed(const Acts::Transform3& transform) {
  
  Acts::Vector3 t_mean       = transform * mean_;
  Acts::Vector3 t_major_axis = transform * major_axis_;
  Acts::Vector3 t_minor_axis = transform * minor_axis_;

  return GaussianDistribution2D(normalization_,
                                t_mean, t_major_axis, t_minor_axis);
  
}

double GaussianDistribution2D::sigma1D(const Acts::Vector3& axis) {

  Acts::Vector3 uaxis = axis / axis.norm();
  return std::sqrt(std::pow(uaxis.dot(major_axis_),2) + std::pow(uaxis.dot(minor_axis_),2) );
}
  
double GaussianDistribution2D::covxy(const Acts::Vector3& xaxis, const Acts::Vector3 yaxis) {

  // Check that the axes are orthogonal
  if (std::abs( xaxis.dot(yaxis) > 1e-9))
    std::cout<<"ERROR:: Pixel axes are not orthogonal"<<std::endl;

  // Find the sin and cos of the angle between the x axis and the major axis
  double cth = (xaxis / xaxis.norm()).dot((major_axis_ / major_axis_.norm()));
  double sth = (yaxis / yaxis.norm()).dot((major_axis_ / major_axis_.norm()));

  // Calculate the x-y covariance matrix element
  return sth * cth * (major_axis_.squaredNorm() - minor_axis_.squaredNorm());
}


double GaussianDistribution2D::upperIntegral1D(const Acts::Vector3& axis, double integration_limit) {
  std::cout<<"UPPER INTEGRAL 1D TO BE IMPLEMENTED"<<std::endl;
  return -999;
}

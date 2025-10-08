#ifndef LDMXSPACEPOINT_H_
#define LDMXSPACEPOINT_H_

// TODO:: Covariance?!
// VarianceR is the variance in the more sensitive direction
// VarianceZ is the variance in the less sensitive direction
// TODO:: Get rid of this class and change it with something that makes some
// sort of sense

// TODO:: The projector should support time as well
// MG Aug 2024  Change all SymMatrix2 to SquareMatrix2 to support ACTs v36
#include <cmath>

//--- Acts ---//
#include "Acts/Definitions/Algebra.hpp"

namespace ldmx {

class LdmxSpacePoint {
 public:
  // Global position constructor with fixed covariance
  LdmxSpacePoint(float x_, float y_, float z_, float t, int layer_) {
    m_x_ = x_;
    m_y_ = y_;
    m_z_ = z_;
    m_t_ = t;
    m_edep_ = 0.;
    m_layer_ = layer_;
    m_id_ = -999;
    m_variance_r_ = 0.050;
    m_variance_z_ = 0.050;
    initialize();
  }

  // Global position constructor with user specified covariance
  LdmxSpacePoint(float x_, float y_, float z_, float t, int layer_, float edep,
                 float vR, float vZ, int id) {
    m_x_ = x_;
    m_y_ = y_;
    m_z_ = z_;
    m_t_ = t;
    m_edep_ = edep;
    m_variance_r_ = vR;
    m_variance_z_ = vZ;
    m_layer_ = layer_;
    m_id_ = id;
    initialize();
  }

  LdmxSpacePoint(const std::vector<float>& gp, float t, int layer_,
                 const std::vector<float>& cv, int id) {
    m_x_ = gp[0];
    m_y_ = gp[1];
    m_z_ = gp[2];
    m_t_ = t;
    m_edep_ = 0.;
    m_layer_ = layer_;
    m_id_ = id;
    m_variance_r_ = cv[0];
    m_variance_z_ = cv[1];
    initialize();
  }

  float x() const { return m_x_; }
  float y() const { return m_y_; }
  float z() const { return m_z_; }
  float t() const { return m_t_; }
  float r() const { return m_r_; }
  float edep() const { return m_edep_; }
  float varianceR() const { return m_variance_r_; }
  float varianceZ() const { return m_variance_z_; }
  int layer() const { return m_layer_; }
  int id() const { return m_id_; }

  void setGlobalPosition(float x_, float y_, float z_) {
    global_pos_.setZero();
    global_pos_(0) = x_;
    global_pos_(1) = y_;
    global_pos_(2) = z_;
  }

  void setLocalPosition(float u, float v) {
    local_pos_.setZero();
    local_pos_(0) = u;
    local_pos_(1) = v;
  }

  void setLocalPosition(const Acts::Vector2& local) { local_pos_ = local; }

  void setLocalCovariance(float vR, float vZ) {
    local_cov_.setZero();
    local_cov_(0, 0) = vR;
    local_cov_(1, 1) = vZ;
  }

  //(1 0 0 0 0 0)
  //(0 1 0 0 0 0)

  void setProjector() {
    projector_.setZero();
    projector_(0, 0) = 1;
    projector_(1, 1) = 1;
  }

  const Acts::SquareMatrix2 getLocalCovariance() const { return local_cov_; };
  const Acts::Vector3 getGlobalPosition() const { return global_pos_; };
  const Acts::Vector2 getLocalPosition() const { return local_pos_; };

  Acts::Vector3 global_pos_;
  Acts::Vector2 local_pos_;
  // TODO:: not sure about this
  Acts::SquareMatrix2 local_cov_;

  // Projection matrix from the full space to the (u,v) space.
  // This can be expanded to (u,v,t) space in the case time needs to be added.

  Acts::ActsMatrix<2, 6> projector_;

 private:
  void initialize() {
    setGlobalPosition(m_x_, m_y_, m_z_);
    m_r_ = std::sqrt(m_x_ * m_x_ + m_y_ * m_y_);
    setLocalCovariance(m_variance_r_, m_variance_z_);
    setProjector();
  };

  float m_x_;
  float m_y_;
  float m_z_;
  float m_t_;
  float m_r_;
  float m_edep_;
  float m_variance_r_;
  float m_variance_z_;
  int m_id_;
  int m_layer_;
};

}  // namespace ldmx

#endif

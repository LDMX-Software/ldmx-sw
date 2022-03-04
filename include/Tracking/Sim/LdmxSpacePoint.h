#ifndef LDMXSPACEPOINT_H_
#define LDMXSPACEPOINT_H_

//TODO:: Covariance?!
//VarianceR is the variance in the more sensitive direction
//VarianceZ is the variance in the less sensitive direction
//TODO:: Get rid of this class and change it with something that makes some sort of sense

//TODO:: The projector should support time as well

#include <cmath>

//--- Acts ---//
#include "Acts/Definitions/Algebra.hpp"

namespace ldmx {

class LdmxSpacePoint  {
        
 public :

  //Global position constructor with fixed covariance
  LdmxSpacePoint(float x, float y,
                 float z, float t,
                 int layer) {
            
    m_x = x;
    m_y = y;
    m_z = z;
    m_t = t;
    m_edep = 0.;
    m_layer = layer;
    m_id = -999;
    m_varianceR = 0.050;
    m_varianceZ = 0.050;
    initialize();
  }

  //Global position constructor with user specified covariance
  LdmxSpacePoint(float x,float y,float z,
                 float t,int layer, float edep,
                 float vR, float vZ,
                 int id) {
    m_x = x;
    m_y = y;
    m_z = z;
    m_t = t;
    m_edep = edep;
    m_varianceR = vR;
    m_varianceZ = vZ;
    m_layer = layer;
    m_id = id;
    initialize();
  }

  /*
    LdmxSpacePoint(Acts::Vector3D gp, float t, int layer,
    Acts::Vector3D cv, int id) {
    m_x = gp(0);
    m_y = gp(1);
    m_z = gp(2);
    m_t = t;
    m_r = std::sqrt(m_x*m_x + m_y*m_y);
    m_layer = layer;
    m_id = id;
    m_varianceR = cv(0);
    m_varianceZ = cv(1);
                        
    }
  */

        
  LdmxSpacePoint(const std::vector<float>& gp, float t, int layer,
                 const std::vector<float>& cv, int id) {

    m_x = gp[0];
    m_y = gp[1];
    m_z = gp[2];
    m_t = t;
    m_edep = 0.;
    m_layer = layer;
    m_id = id;
    m_varianceR = cv[0];
    m_varianceZ = cv[1];
    initialize();
  }
        
        
  float x() const {return m_x;}
  float y() const {return m_y;}
  float z() const {return m_z;}
  float t() const {return m_t;}
  float r() const {return m_r;}
  float edep() const {return m_edep;}
  float varianceR() const {return m_varianceR;}
  float varianceZ() const {return m_varianceZ;}
  int   layer() const {return m_layer;}
  int   id() const {return m_id;}

  
  void setGlobalPosition(float x, float y, float z) {
    global_pos_.setZero();
    global_pos_(0)=x;
    global_pos_(1)=y;
    global_pos_(2)=z;
  }

  void setLocalPosition(float u, float v) {
    local_pos_.setZero();
    local_pos_(0)=u;
    local_pos_(1)=v;
  }

  void setLocalPosition(const Acts::Vector2& local) {
    local_pos_ = local;
  }

  void setLocalCovariance(float vR, float vZ) {
    local_cov_.setZero();
    local_cov_(0,0) = vR;
    local_cov_(1,1) = vZ;
    
  }

  //(1 0 0 0 0 0)
  //(0 1 0 0 0 0)
  
  void setProjector() {
    projector_.setZero();
    projector_(0,0) = 1;
    projector_(1,1) = 1;
  }
    
  const Acts::SymMatrix2 getLocalCovariance(){return local_cov_;};
  const Acts::Vector3 getGlobalPosition(){return global_pos_;};
  const Acts::Vector2 getLocalPosition(){return local_pos_;};
  
  Acts::Vector3 global_pos_;
  Acts::Vector2 local_pos_;
  //TODO:: not sure about this
  Acts::SymMatrix2 local_cov_;
  
  //Projection matrix from the full space to the (u,v) space.
  //This can be expanded to (u,v,t) space in the case time needs to be added.

  Acts::ActsMatrix<2,6> projector_;
  
 private:


  void initialize() {
    setGlobalPosition(m_x,m_y,m_z);
    m_r = std::sqrt(m_x*m_x + m_y*m_y);
    setLocalCovariance(m_varianceR, m_varianceZ);
    setProjector();
  };
  
  float m_x;
  float m_y;
  float m_z;
  float m_t;
  float m_r;
  float m_edep;
  float m_varianceR;
  float m_varianceZ;
  int   m_id;
  int   m_surfaceId;
  int   m_layer;

  

  
};    
    
        

    


}

#endif

#ifndef TRACKING_EVENT_TRACK_H_
#define TRACKING_EVENT_TRACK_H_


//----------------------//
//   C++ Standard Lib   //
//----------------------//
#include <iostream>
#include <vector>

//----------//
//   ROOT   //
//----------//
#include "TObject.h"


// --- ACTS --- //
//#include "Acts/Definitions/TrackParametrization.hpp"
//#include "Acts/EventData/TrackParameters.hpp"


namespace ldmx {

/**
 * Implementation of a track object.
 *
 * This class encapsulates all the information of a particle trajectory in the tracker
 *
 */

class Track {
 public:
  
  Track(){};
        
  /**
   * Destructor.
   *
   * Currently, the destructor does nothing.
   */
  virtual ~Track(){};
    
  /**
   * Print the string representation of this object. 
   *
   * This class is needed by ROOT when building the dictionary. 
   */
  void Print() const;
    
  void setNhits(int nhits) {n_hits_ = nhits;}
  int getNhits() const { return n_hits_;}

  void setNoutliers(int nout) {n_outliers_ = nout;}
  void setNdf(int ndf) {ndf_ = ndf;}
  void setNsharedHits(int nsh) {n_shared_hits_ = nsh;}
  
  
  void setChi2(double chi2) {chi2_ = chi2;} 
  double getChi2() const { return chi2_;}

  //in units of e 
  int q() const {return perigee_pars_[4] > 0 ? 1 : -1;}
  
  
  ///d_0 z_0 phi_0 theta q/p t
  //void setPerigeeParameters(const Acts::BoundVector& par)  {perigee_pars_ = par; }
  //Acts::BoundVector getPerigeeParameters() {return perigee_pars_;}

  //void setPerigeeCov(const Acts::BoundMatrix& cov) {perigee_cov_ = cov;}
  //Acts::BoundMatrix getPerigeeCov() {return perigee_cov_;}

  //void setPerigeeState(const Acts::BoundVector& par, const Acts::BoundMatrix& cov) {
  //  perigee_pars_ = par;
  //  perigee_cov_  = cov;
  //}

  //Vector representation
  void setPerigeeParameters(const std::vector<double>& par) {perigee_pars_ = par;}
  std::vector<double>  getPerigeeParameters() const {return perigee_pars_;}

  void setPerigeeCov(const std::vector<double>& cov) {perigee_cov_ = cov;}
  std::vector<double> getPerigeeCov() const {return perigee_cov_;}

  void setPerigeeLocation(const std::vector<double>& perigee) {
    perigee_ = perigee;
  }
  
  void setPerigeeLocation(const double& x, const double& y, const double& z) {
    perigee_[0] = x;
    perigee_[1] = y;
    perigee_[2] = z;
  }

  std::vector<double> getPerigeeLocation() const {return perigee_;};
  double getPerigeeX() const {return perigee_[0];};
  double getPerigeeY() const {return perigee_[1];};
  double getPerigeeZ() const {return perigee_[2];};
    
  //getters -- TODO use an enum instead
  
  double getD0()    const {return perigee_pars_[0];};
  double getZ0()    const {return perigee_pars_[1];};
  double getPhi()   const {return perigee_pars_[2];};
  double getTheta() const {return perigee_pars_[3];};
  double getQoP()   const {return perigee_pars_[4];};
  double getT()     const {return perigee_pars_[5];};
  
  
 protected:
    
  int n_hits_{0};
  int n_outliers_{0};
  int ndf_{0};
  int n_shared_hits_{0};
  

  //particle hypothesis if truth track
  //int pdgID_{0};
  
  double chi2_{0};
    
  //The parameters and covariance matrix wrt the perigee surface
  //Acts::BoundVector perigee_pars_;
  //Acts::BoundSymMatrix perigee_cov_;

  //6 elements
  //d0 / z0 / phi / theta / qop / t
  std::vector<double> perigee_pars_{0.,0.,0.,0.,0.,0.};

  //21 elements
  //d0d0 d0z0 d0phi d0th  d0qop  d0t
  //     z0z0 z0phi z0th  z0qop  z0t
  //          phph  phith phqop  pht
  //                 thth thqop  tht
  //                      qopqop qopt
  //                             t
  std::vector<double> perigee_cov_;
  
  //The perigee location
  std::vector<double> perigee_{0.,0.,0.};
  
  ///Class declaration needed by the ROOT disctionary.
  ClassDef(Track, 1);
    
}; //Track
}//namespace ldmx
  
#endif // TRACKING_EVENT_TRACK_H_

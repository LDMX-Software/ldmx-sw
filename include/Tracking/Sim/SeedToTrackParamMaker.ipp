#include "SeedToTrackParamMaker.h"
#include <cmath>

namespace tracking {
namespace sim {
        
/// V. Karimaki NIM A305 (1991) 187-191
/// In Karimaki's fit, d0 is the distance of the closest approach to the origin, rho (=1/R) is the curvature, 
/// phi is the angle of the direction propagation (counter clock wise as positive)
        
template <typename external_spacepoint_t>
bool SeedToTrackParamMaker::KarimakiFit(const std::vector<external_spacepoint_t*>&sp, std::array<double,9>& data, const Acts::Vector2 refPoint) {
        
  if (sp.size() < 3)
    return false;
            
  double Sx2 =0., x2m=0. ;  // <x^2>
  double Sx  =0., xm =0. ;  // <x>
  double Sxy =0., xym=0. ;  // <xy>
  double Sy2 =0., y2m=0. ;  // <y^2>
  double Sy  =0., ym =0. ;  // <y>
  double Sr2 =0., r2m=0. ;  // <r^2>
  double Sr4 =0., r4m=0. ;  // <r^4>
  double Sxr2=0., xr2m=0.;  // <xr^2>
  double Syr2=0., yr2m=0.;  // <yr^2>
  double Sw  =0.;  // sum of weights
            
  int n = sp.size();
            
  for (size_t i_sp = 0; i_sp<sp.size(); i_sp++) {
                
    double x  = sp[i_sp]->x() - refPoint[0];
    double y  = sp[i_sp]->y() - refPoint[1];
    double r2 = x*x+y*y;
                
    //The weight is 1/rphi error. 
    //If a point has error equal to 0, then the weight is set to 1.e8 (very strong constraint)
    //Setting the weight to -1, ignores the point
                
    double rphi_err = std::sqrt(sp[i_sp]->varianceR());            
    double w = 1.;
                
    if (rphi_err < 0.) {
      w=0.;
    }
                
    else if (rphi_err < 1e-8) {
      w=1e8;
    }
                
    else {
      w = 1./rphi_err;
    }
                
    Sw   += w;
    Sx2  += x*x   * w;
    Sx   += x     * w;
    Sxy  += x*y   * w;
    Sy2  += y*y   * w;
    Sy   += y     * w;
    Sr2  += r2    * w;
    Sr4  += r2*r2 * w;
    Sxr2 += x*r2  * w;
    Syr2 += y*r2  * w;
                
  }
            
  x2m  = Sx2  / Sw;
  xm   = Sx   / Sw;
  xym  = Sxy  / Sw;
  y2m  = Sy2  / Sw;
  ym   = Sy   / Sw;
  r2m  = Sr2  / Sw;
  r4m  = Sr4  / Sw;
  xr2m = Sxr2 / Sw;
  yr2m = Syr2 / Sw;
            
  double Cxx   = x2m  - xm*xm;
  double Cxy   = xym  - xm*ym;
  double Cyy   = y2m  - ym*ym;
  double Cxr2  = xr2m - xm*r2m; 
  double Cyr2  = yr2m - ym*r2m;
  double Cr2r2 = r4m  - r2m*r2m;
            
  double q1 = Cr2r2*Cxy - Cxr2*Cyr2 ;
  double q2 = Cr2r2*(Cxx-Cyy) - Cxr2*Cxr2+Cyr2*Cyr2;
            
  double phi   = 0.5 * atan(2*q1/q2);
  double k     = (sin(phi)*Cxr2 - cos(phi)*Cyr2) * (1./Cr2r2);
  double delta = -k*r2m + sin(phi)*xm - cos(phi)*ym;
            
  double rho = (2*k) / (sqrt(1-4*delta*k));
  double d   = (2*delta) / (1+ sqrt(1-4*delta*k));
            
  Acts::Vector2 r1{sp[0]->x() - refPoint[0], sp[0]->y() - refPoint[1]};
  Acts::Vector2 r2{sp[1]->x() - refPoint[0], sp[1]->y() - refPoint[1]};
            
  transformRhoPhid(r1,r2,phi,rho,d);
            
  // phi between [-pi,pi)
  if (phi>M_PI)
    phi += -2*M_PI;

  //Compute the covariance matrix. 
  //The order of the covariance matrix is rho,phi,d -> par1,par2,par3
  //(V-1)jk = sum_i w_i * depsilon_i/dparj *depsilon_i/dpark
            
  double u    = 1 + rho*d;
  double sphi = sin(phi);
  double cphi = sqrt(1-sphi*sphi);
            
  double Chi2 = Sw*(1.+rho*d)*(1.+rho*d)*(sphi*sphi*Cxx - 2*sphi*cphi*Cxy + cphi*cphi*Cyy - k*k*Cr2r2);
                            
  double Salpha = sphi*Sx   - cphi*Sy;
  double Sbeta  = cphi*Sx   + sphi*Sy;
  double Sgamma = (sphi*sphi - cphi*cphi)*Sxy + sphi*cphi*(Sx2 - Sy2);
  double Sdelta = sphi*Sxr2 - cphi*Syr2;
  double Saa    = sphi*sphi*Sx2 -2*sphi*cphi*Sxy + cphi*cphi*Sy2;
            
  double Vrr_m1     = (1./4.) * Sr4 - d*(Sdelta -d*(Saa + (1./2.)*Sr2 - d*(Salpha - (1./4.)*d*Sw)));
  double Vrphi_m1   = -u * ( (1./2.) * (cphi*Sxr2 + sphi*Syr2) - d*(Sgamma - (1./2.)*d*Sbeta));
  double Vphiphi_m1 = u*u * (cphi*cphi*Sx2 + sin(2*phi)*Sxy + sphi*sphi*Sy2);
  double Vrhod_m1   = rho * (-(1./2.)*Sdelta + d*Saa) + (1./2.)*u*Sr2 - (1./2.)*d*((2*u+rho*d)*Salpha - u*d*Sw);
  double Vphid_m1   = u*(rho*Sgamma - u*Sbeta);
  double Vdd_m1     = rho*(rho*Saa - 2*u*Salpha)+u*u*Sw;
            
  Acts::SymMatrix3 kariCov_inv;
            
  //TODO:: This one should be fixed
  kariCov_inv(0,0) = Vrr_m1;
  kariCov_inv(0,1) = Vrphi_m1;
  kariCov_inv(1,0) = Vrphi_m1;
            
  kariCov_inv(1,1) = Vphiphi_m1;
            
  kariCov_inv(1,2) = Vphid_m1;
  kariCov_inv(2,1) = Vphid_m1;
            
  kariCov_inv(2,0) = Vrhod_m1;         
  kariCov_inv(0,2) = Vrhod_m1;
  kariCov_inv(2,2) = Vdd_m1;
            
  Acts::SymMatrix3 kariCov = kariCov_inv.inverse();
            
  //Compute the corrections to the estimated parameters
  double sigma = -rho*Sdelta + 2*u*Saa - d*(1+u)*Salpha;
  double dChi2drho = d*sigma;
  double dChi2dd   = rho*sigma;
            
  double delta_rho = (-1./2.) * (kariCov(0,0)*dChi2drho + kariCov(0,2)*dChi2dd);
  double delta_phi = (-1./2.) * (kariCov(1,0)*dChi2drho + kariCov(1,2)*dChi2dd);
  double delta_d   = (-1./2.) * (kariCov(2,0)*dChi2drho + kariCov(2,2)*dChi2dd);
            
  rho += delta_rho;
  phi += delta_phi;
  d   += delta_d;
  
  std::cout<<"rho:"<<rho<<" phi:"<<phi<<" d:"<<d<<" Chi2:"<<Chi2<<std::endl;

  data[0]=rho;
  data[1]=phi;
  data[2]=d;
            
  return true;
            
}
    
/// see https://acode-browser.usatlas.bnl.gov/lxr/source/athena/InnerDetector/InDetRecTools/SiTrackMakerTool_xk/src/SiTrackMaker_xk.cxx
template <typename external_spacepoint_t>
bool SeedToTrackParamMaker::FitSeedAtlas(const Acts::Seed<external_spacepoint_t>& seed, std::array<double,9>& data, const Acts::Transform3& Tp, const double& bFieldZ) {
  return FitSeedAtlas(seed.sp(),data, Tp, bFieldZ);
}

        
//double H = 0.0015; //kTesla 

/// This method gives an estimate of the track parameters of a seed using a conformal map transformation
/// The track parameters are of the form l1, l2, phi, theta, q/p or d0, z0, phi, theta, q/p. 
/// phi0 is the angle of the track direction with respect the origin, positive when counter clock-wise
    
    
template <typename external_spacepoint_t>
bool SeedToTrackParamMaker::FitSeedAtlas(const std::vector<external_spacepoint_t>& sp, std::array<double,9>& data, const Acts::Transform3& Tp, const double& bFieldZ) {
        
  double pTmin = 0.1;
        
  /// Define the locations of the space points with respect to the first one in the Space point vector
        
  double x0 = sp[0]->x();
  double y0 = sp[0]->y();
  double z0 = sp[0]->z();

  double r0 = sqrt(x0*x0+y0*y0);
        
  double x1 = sp[1]->x() - x0;
  double y1 = sp[1]->y() - y0;
  double x2 = sp[2]->x() - x0;
  double y2 = sp[2]->y() - y0;
  double z2 = sp[2]->z() - z0; 
        

  /// Define conformal map variables
  double u1 = 1./sqrt(x1*x1+y1*y1)       ;
  double rn = x2*x2+y2*y2                ;
  double r2 = 1./rn                      ;
        
  /// a = cos(phi_0), b = sin(phi_0) in some other notations
  /// and solve
  double a  = x1*u1                      ;
  double b  = y1*u1                      ;
  double u2 = (a*x2+b*y2)*r2             ;
  double v2 = (a*y2-b*x2)*r2             ;
  double A  = v2/(u2-u1)                 ;
  double B  = 2.*(v2-A*u2)               ;
        
  /// 1/helixradius = C
  double C  = B/sqrt(1.+A*A)             ;
                
  double T  = z2*sqrt(r2)/(1.+.04*C*C*rn);
        
  /// Project to the surface
  double Ax[3] = {Tp(0,0),Tp(1,0),Tp(2,0)};
  double Ay[3] = {Tp(0,1),Tp(1,1),Tp(2,1)};
  double D [3] = {Tp(0,3),Tp(1,3),Tp(2,3)};
        
  double d[3]  = {x0-D[0], y0-D[1], z0-D[2]};
        
  /// l1 is the (most) sensitive direction, l2 is the un- (less) sensitive direction
  data[0] = d[0]*Ax[0]+d[1]*Ax[1]+d[2]*Ax[2]; 
  data[1] = d[0]*Ay[0]+d[1]*Ay[1]+d[2]*Ay[2]; 
        
  data[2] = std::atan2(b+a*A,a-b*A);
  data[3] = std::atan2(1.,T);
  data[5] = -C / (300. * bFieldZ);
        
  /// B in this computation is twice the usual B
  double b_c = 1/B;
  double a_c = -1*b_c*A;
  double R = 1./C;
        
  //std::cout<<"(a_c, b_c) = "<<a_c<<","<<b_c<<std::endl;
  //std::cout<<"R="<<R<<std::endl;
        
  // wrt real origin
  double a_cp = a_c*(a) - b_c*(b) + x0;
  double b_cp = b_c*(a) + a_c*(b) + y0;
  double ip = (1./(2.*R))*(a_cp*a_cp+b_cp*b_cp -R*R);
        
  //std::cout<<"SeedToTrackParamMaker (a_cp,b_cp)="<<a_cp<<","<<b_cp<<std::endl;
  //std::cout<<"Ip="<<ip<<std::endl;
        
  /// Return false if the transverse momentum is less than 90% the minimum momentum
  if (fabs(data[5]*pTmin) > 1.1)
    return false;
        
  /// Momentum is given by pT/cos(theta)
  data[4] = data[5] / std::sqrt(1. + T*T);

  /// Store the reference point
  data[6] = x0;
  data[7] = y0;
  data[8] = z0;
        
  return true;
}
    
template <typename external_spacepoint_t>
bool SeedToTrackParamMaker::FitSeedLinPar (const Acts::Seed<external_spacepoint_t>& seed, std::vector<double>& data)  {
  return true;
}

}        
}

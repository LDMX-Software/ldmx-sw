// Exact ray chords in convex vertical prisms. Units: mm, g/cm^3 -> g/cm^2.
// The caller supplies installed geometry and layer assignments; no transport.
#include <algorithm>
#include <cmath>
#include <cstdint>

extern "C" void trace_prisms(
    int64_t nray, const double* xy, const double* slope, double zref,
    int nprism, const double* prisms, const double* planes,
    int nlayer, const double* layer_zlow,
    uint32_t* masks, double* columns, double* cumulative) {
  // Per prism: xmin,xmax,ymin,ymax,zmin,zmax,rho,layer,plane_start,nplanes.
  double stop[32];
  for (int j=0;j<nlayer;++j) stop[j]=zref-layer_zlow[j];
  for (int64_t i=0;i<nray;++i) {
    const double x=xy[2*i], y=xy[2*i+1], sx=slope[2*i], sy=slope[2*i+1];
    const double pathfactor=std::sqrt(1+sx*sx+sy*sy)/10.;
    double delta[33]={0}, partial[32]={0}, total=0;
    uint32_t bits=0;
    for (int p=0;p<nprism;++p) {
      const double* g=prisms+10*p;
      double enter=zref-g[5], leave=zref-g[4];
      // Fast bounding rectangle rejects most small devices for broad rays.
      bool hit=true;
      if (std::abs(sx)<1e-14) { if(x<g[0]-1e-8||x>g[1]+1e-8) hit=false; }
      else {double a=(g[0]-x)/sx,b=(g[1]-x)/sx;if(a>b)std::swap(a,b);enter=std::max(enter,a);leave=std::min(leave,b);}
      if(!hit||leave<=enter+1e-10)continue;
      if (std::abs(sy)<1e-14) { if(y<g[2]-1e-8||y>g[3]+1e-8) hit=false; }
      else {double a=(g[2]-y)/sy,b=(g[3]-y)/sy;if(a>b)std::swap(a,b);enter=std::max(enter,a);leave=std::min(leave,b);}
      if(!hit||leave<=enter+1e-10)continue;
      const int first=static_cast<int>(g[8]), count=static_cast<int>(g[9]);
      for(int k=0;k<count;++k){
        const double* e=planes+3*(first+k);
        const double val=e[0]*x+e[1]*y+e[2], speed=e[0]*sx+e[1]*sy;
        if(std::abs(speed)<1e-14){if(val>1e-8){hit=false;break;}}
        else {const double t=-val/speed;if(speed>0)leave=std::min(leave,t);else enter=std::max(enter,t);}
        if(leave<=enter+1e-10){hit=false;break;}
      }
      if(!hit||leave<=enter+1e-10)continue;
      const int layer=static_cast<int>(g[7]);
      if(layer>=0)bits|=(uint32_t(1)<<layer);
      if(g[6]<=0)continue;
      const double factor=pathfactor*g[6], full=(leave-enter)*factor;
      total+=full;
      const int begin=std::lower_bound(stop,stop+nlayer,enter)-stop;
      const int end=std::lower_bound(stop,stop+nlayer,leave)-stop;
      for(int j=begin;j<end;++j)partial[j]+=(stop[j]-enter)*factor;
      if(end<nlayer)delta[end]+=full;
    }
    masks[i]=bits;columns[i]=total;
    double running=0;
    for(int j=0;j<nlayer;++j){running+=delta[j];cumulative[i*nlayer+j]=running+partial[j];}
  }
}

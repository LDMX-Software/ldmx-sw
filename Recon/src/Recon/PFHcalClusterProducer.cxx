#include "Recon/PFHcalClusterProducer.h"

#include "Hcal/Event/HcalHit.h"
#include "Hcal/Event/HcalCluster.h"
#include "TGraph.h"
#include "TFitResult.h"

using std::cout;
using std::endl;

namespace recon {

// void fillClusterInfo(ldmx::HcalCluster &cl, std::vector<ldmx::HcalHit> hits){
//   float e(0),x(0),y(0),z(0),xx(0),yy(0),zz(0),n(0);
//   float w = 1; // weight
//   float sumw = 0;
//   float minEnergy=0; // min hit energy to consider
//   std::vector<float> xvals{};
//   std::vector<float> yvals{};
//   std::vector<float> zvals{};
//   for(const auto &h : hits){
//     //if (h.getEnergy() < minEnergy) continue;
//     // w = log( h.getEnergy() - log(minEnergy) );
//     e += h.getEnergy();
//     x += w * h.getXPos();
//     y += w * h.getYPos();
//     z += w * h.getZPos();
//     xx += w * h.getXPos() * h.getXPos();
//     yy += w * h.getYPos() * h.getYPos();
//     zz += w * h.getZPos() * h.getZPos();
//     n += 1;
//     sumw += w;
//     xvals.push_back(x);
//     yvals.push_back(y);
//     zvals.push_back(z);
//   }
//   x /= sumw; // now is <x>
//   y /= sumw;
//   z /= sumw;
//   xx /= sumw; // now is <x^2>
//   yy /= sumw;
//   zz /= sumw;
//   xx = sqrt(xx - x * x); //now is sqrt(<x^2>-<x>^2)
//   yy = sqrt(yy - y * y); 
//   zz = sqrt(zz - z * z);
//   cl.setEnergy(e);
//   cl.setNHits(n);
//   cl.setCentroidXYZ(x,y,z);
//   cl.setRMSXYZ(xx,yy,zz);

//   if (xvals.size()>2){
//     for(int i=0; i<xvals.size();i++){ // mean subtract
//       xvals[i] = xvals[i] - x;
//       yvals[i] = yvals[i] - y;
//       zvals[i] = zvals[i] - z;
//     }
//     TGraph gxz(zvals.size(), zvals.data(), xvals.data());
//     auto r_xz = gxz.Fit("pol1","SQ"); // p0 + x*p1
//     cl.setDXDZ( r_xz->Value(1) );
//     cl.setEDXDZ( r_xz->ParError(1) );
//     TGraph gyz(zvals.size(), zvals.data(), yvals.data());
//     auto r_yz = gyz.Fit("pol1","SQ"); // p0 + x*p1
//     cl.setDYDZ( r_yz->Value(1) );
//     cl.setEDYDZ( r_yz->ParError(1) );
//   }
//   return;
//   //pfClusters.push_back(cl);
// }


template <class C, class H>
void fillClusterInfoFromHits2(C &cl, std::vector<H*> hits,
			      float minHitEnergy=0, bool logEnergyWeight=true){
  float e(0),x(0),y(0),z(0),xx(0),yy(0),zz(0),n(0);
  float w = 1; // weight
  float sumw = 0;
  std::vector<float> xvals{};
  std::vector<float> yvals{};
  std::vector<float> zvals{};
  // if (!select){ // use all the hit indices...
  //   for(unsigned int i=0; i<hits.size();i++){
  //     toUse.push_back(i);
  //   }
  // }
  for(const H* h : hits){
    // const auto h = hits[i];
    if (h->getEnergy() < minHitEnergy) continue;
    if (logEnergyWeight) w = log( h->getEnergy() - log(minHitEnergy) );
    e += h->getEnergy();
    x += w * h->getXPos();
    y += w * h->getYPos();
    z += w * h->getZPos();
    xx += w * h->getXPos() * h->getXPos();
    yy += w * h->getYPos() * h->getYPos();
    zz += w * h->getZPos() * h->getZPos();
    n += 1;
    sumw += w;
    xvals.push_back(x);
    yvals.push_back(y);
    zvals.push_back(z);
  }
  x /= sumw; // now is <x>
  y /= sumw;
  z /= sumw;
  xx /= sumw; // now is <x^2>
  yy /= sumw;
  zz /= sumw;
  xx = sqrt(xx - x * x); //now is sqrt(<x^2>-<x>^2)
  yy = sqrt(yy - y * y); 
  zz = sqrt(zz - z * z);
  cl.setEnergy(e);
  cl.setNHits(n);
  cl.setCentroidXYZ(x,y,z);
  cl.setRMSXYZ(xx,yy,zz);

  if (xvals.size()>2){
    for(int i=0; i<xvals.size();i++){ // mean subtract
      xvals[i] = xvals[i] - x;
      yvals[i] = yvals[i] - y;
      zvals[i] = zvals[i] - z;
    }
    TGraph gxz(zvals.size(), zvals.data(), xvals.data());
    auto r_xz = gxz.Fit("pol1","SQ"); // p0 + x*p1
    cl.setDXDZ( r_xz->Value(1) );
    cl.setEDXDZ( r_xz->ParError(1) );
    TGraph gyz(zvals.size(), zvals.data(), yvals.data());
    auto r_yz = gyz.Fit("pol1","SQ"); // p0 + x*p1
    cl.setDYDZ( r_yz->Value(1) );
    cl.setEDYDZ( r_yz->ParError(1) );
  }
  return;
  //pfClusters.push_back(cl);
}

bool isIn2(unsigned int i, std::vector<unsigned int> l){
  return std::find(l.begin(), l.end(), i) != l.end();
}

float dist(ldmx::HcalHit a, ldmx::HcalHit b){
  return sqrt( pow(a.getXPos() - b.getXPos(),2)
	       + pow(a.getYPos() - b.getYPos(),2)
	       + pow(a.getZPos() - b.getZPos(),2));
}
  
template <class T>
std::vector<std::vector<const T*> > runDBSCAN2( const std::vector<T> &hits,
		float minEnergy=0, int minNHit=2, float maxDist=100,
		bool debug = false){
  
    const int n = hits.size();
    std::vector<std::vector<const T*> > idx_clusters;
    std::vector<unsigned int> tried; tried.reserve(n);
    std::vector<unsigned int> used; used.reserve(n);
    for(unsigned int i=0;i<n;i++){
      if( isIn2(i,tried) ) continue;
      tried.push_back(i);
      if (debug) cout << "trying " << i << endl;
      if (hits[i].getEnergy() < minEnergy) continue;
      std::set<unsigned int> neighbors;
      unsigned int nNearby=1;
      // find neighbors
      for(unsigned int j=0;j<n;j++){
	if( i!=j && dist(hits[i],hits[j]) < maxDist){
	  neighbors.insert(j);
	  if (hits[j].getEnergy() >= minEnergy) nNearby++;
	}
      }
      if (nNearby >= minNHit){
	std::vector<const T*> idx_cluster{&hits[i]}; // start a cluster
	used.push_back(i);
	if (debug) cout << "- starting a cluster from " << i << endl;
	for(unsigned int j : neighbors){
	  if ( !isIn2(j,tried) ){
	    tried.push_back(j);
	    if (debug) cout << "== tried " << j << endl;
	    std::vector<unsigned int> neighbors2;
	    for(unsigned int k=0;k<n;k++){
	      if( dist(hits[k],hits[j]) < maxDist){
		neighbors2.push_back(k);
	      }
	    }
	    for(unsigned int k : neighbors2)
	      neighbors.insert(k);
	  }
	  if ( !isIn2(j,used) ) {
	    if (debug) cout << "== used " << j << endl;
	    used.push_back(j);
	    idx_cluster.push_back(&hits[j]);
	  }
	}
	// if (debug){ 
	//   cout << "adding cluster: {";
	//   for (auto i : idx_cluster) cout << i << " ";
	//   cout << "}" << endl;
	// }
	idx_clusters.push_back( idx_cluster );
      }
    }
    return idx_clusters;
    if (debug)
      cout << "done. writing this many clusters out: " << idx_clusters.size() << endl;
}


void PFHcalClusterProducer::configure(framework::config::Parameters& ps) {
  hitCollName_ = ps.getParameter<std::string>("hitCollName");
  clusterCollName_ = ps.getParameter<std::string>("clusterCollName"); 
  singleCluster_ = ps.getParameter<bool>("doSingleCluster");
  logEnergyWeight_ = ps.getParameter<bool>("logEnergyWeight");
  minClusterHitMult_ = ps.getParameter<int>("minClusterHitMult");
  clusterHitDist_ = ps.getParameter<double>("clusterHitDist");
  minHitEnergy_ = ps.getParameter<double>("minHitEnergy");
}

void PFHcalClusterProducer::produce(framework::Event& event) {

  if (!event.exists(hitCollName_)) return;
  const auto hcalRecHits = event.getCollection<ldmx::HcalHit>(hitCollName_);
  //std::cout << "found " << hcalRecHits.size() << " hits" << std::endl;

  std::vector<ldmx::HcalCluster> pfClusters;
  if(!singleCluster_){ 
    std::vector<std::vector<const ldmx::HcalHit*> > all_hit_ptrs = runDBSCAN2(hcalRecHits, 
        minHitEnergy_, minClusterHitMult_, clusterHitDist_);
    for(const auto hit_ptrs : all_hit_ptrs){
      ldmx::HcalCluster cl;
      fillClusterInfoFromHits2(cl, hit_ptrs, minHitEnergy_, logEnergyWeight_);
      pfClusters.push_back(cl);
    }    
  } else {
    ldmx::HcalCluster cl;
    // fillClusterInfo(cl,hcalRecHits);
    std::vector<const ldmx::HcalHit*> ptrs; 
    ptrs.reserve(hcalRecHits.size());
    for (const auto &h : hcalRecHits) 
      ptrs.push_back(&h);
    fillClusterInfoFromHits2(cl, ptrs, minHitEnergy_, logEnergyWeight_);
    pfClusters.push_back(cl);
  }
  event.add(clusterCollName_, pfClusters);
}

void PFHcalClusterProducer::onFileOpen() {
  ldmx_log(debug) << "Opening file!";

  return;
}

void PFHcalClusterProducer::onFileClose() {
  ldmx_log(debug) << "Closing file!";

  return;
}

void PFHcalClusterProducer::onProcessStart() {
  ldmx_log(debug) << "Process starts!";

  return;
}

void PFHcalClusterProducer::onProcessEnd() {
  ldmx_log(debug) << "Process ends!";

  return;
}

}  // namespace recon

DECLARE_PRODUCER_NS(recon, PFHcalClusterProducer);
